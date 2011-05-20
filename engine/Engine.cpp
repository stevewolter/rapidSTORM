#define DSTORM_ENGINE_CPP

#include "debug.h"

#include "EngineDebug.h"
#include "engine/Engine.h"

#include <dStorm/data-c++/Vector.h>
#include <cassert>

#include <dStorm/input/Source_impl.h>
#include <dStorm/engine/SpotFinder.h>
#include <dStorm/engine/SpotFitterFactory.h>
#include <dStorm/engine/SpotFitter.h>
#include <dStorm/engine/Config.h>
#include <dStorm/output/Traits.h>
#include <dStorm/output/LocalizedImage_traits.h>
#include <dStorm/output/LocalizedImage.h>
#include <dStorm/engine/JobInfo.h>
#include <dStorm/image/constructors.h>
#include <dStorm/image/slice.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include "PlaneFlattener.h"

#ifdef DSTORM_MEASURE_TIMES
#include <time.h>
clock_t smooth_time = 0, search_time = 0, fit_time = 0;
#endif

static double background_variance = 0;

namespace dStorm {
namespace engine {

Engine::Engine(
    Config &config, 
    std::auto_ptr<Input> input
)
: Base(*this, input->flags ),
  Object("EngineStatus", "Computation status"),
  input(input), 
  config(config),
  errors("ErrorCount", "Number of dropped images", 0)
{
    DEBUG("Constructing engine");
    DEBUG("Spot fitter is named " << this->config.spotFittingMethod().getNode().getName());

    errors.editable = false;
    errors.viewable = false;

    push_back( *this->input );
    push_back( errors );
}

Engine::~Engine() {
    DEBUG("Destructing engine");
    input.reset();
    DEBUG("Destructed engine");
}

boost::shared_ptr< input::Traits<output::LocalizedImage> >
Engine::convert_traits( Config& config, boost::shared_ptr< const input::Traits<engine::Image> > imProp )
{
    input::Traits<Localization> rv( *imProp );
    DEBUG("Getting other traits dimensionality");
    DEBUG("Getting minimum amplitude");
    if ( config.amplitude_threshold().is_set() )
        rv.amplitude().range().first = *config.amplitude_threshold();
    rv.fluorophore().is_given = imProp->fluorophores.size() > 1;
    rv.fluorophore().range().first = 0;
    rv.fluorophore().range().second = imProp->fluorophores.size() - 1;

    boost::shared_ptr< input::Traits<output::LocalizedImage> > rvt( new TraitsPtr::element_type( rv ) );
    rvt->source_image_is_set = true;
    rvt->smoothed_image_is_set = true;
    rvt->candidate_tree_is_set = true;
    rvt->input_image_traits.reset( imProp->clone() );

    return rvt;
}

Engine::TraitsPtr Engine::get_traits() {
    DEBUG("Retrieving input traits");
    if ( imProp.get() == NULL )
        imProp = input->get_traits();

    if ( ! config.amplitude_threshold().is_set() ) {
        DEBUG("Guessing input threshold");
        if ( imProp->background_stddev.is_set() ) {
            config.amplitude_threshold = 35.0f * (*imProp->background_stddev);
        } else {
            throw std::runtime_error("Amplitude threshold is not set and could not be determined from background noise strength");
        }
        DEBUG("Guessed amplitude threshold " << *config.amplitude_threshold());
    } else {
        DEBUG("Using amplitude threshold " << *config.amplitude_threshold());
    }

    input::Traits<output::LocalizedImage>::Ptr prv =
        convert_traits(config, imProp);
    prv->carburettor = input.get();
    prv->image_number().is_given = true;
    prv->engine = this;

    DEBUG("Setting traits from spot fitter");
    for (unsigned int fluorophore = 0; fluorophore < imProp->fluorophores.size(); ++fluorophore) {
        JobInfo info(config, *imProp, fluorophore);
        config.spotFittingMethod().set_traits( *prv, info );
    }
    DEBUG("Returning traits");

    return prv;
}

void Engine::dispatch(Messages m) {
    if ( m.test( RepeatInput ) ) {
        DEBUG("Engine is restarting");
        errors = 0;
    }
    upstream().dispatch(m);
}

class Engine::_iterator
: public boost::iterator_facade< 
    _iterator, 
    output::LocalizedImage,
    std::input_iterator_tag>
{
    Input::iterator base;

    Engine& engine;

    class WorkHorse;
    mutable std::auto_ptr<WorkHorse> work_horse;
    mutable bool did_compute;

    /*=== iterator_facade interface ===*/
    output::LocalizedImage& dereference() const; 
    bool equal(const _iterator& o) const { 
        DEBUG("Comparing engine iterators"); 
        bool rv = ( base == o.base ); 
        DEBUG("Compared engine iterators with " << rv); 
        return rv;
    }
    void increment() { DEBUG("Incrementing engine iterators"); did_compute = false; ++base; }

  public:
    _iterator( Engine& engine, Input::iterator base );
    _iterator( const _iterator& );
    ~_iterator();

    friend class boost::iterator_core_access;
};

class Engine::_iterator::WorkHorse {
    Engine& engine;
    Config& config;

    int maximumLimit;
    PlaneFlattener flattener;
    std::auto_ptr<spot_finder::Base> finder;
    boost::ptr_vector<spot_fitter::Implementation> fitter;
    data_cpp::Vector<Localization> buffer;
    CandidateTree<SmoothedPixel> maximums;
    int origMotivation;

  public:
    WorkHorse( Engine& engine );
    ~WorkHorse() {
        DEBUG("Destructing spot finder");
        finder.reset();
        DEBUG("Destructing spot fitters");
        fitter.clear();
        DEBUG("Destructing rest");
    }
    void compute( Input::iterator image );

    output::LocalizedImage resultStructure;
};

output::LocalizedImage& Engine::_iterator::dereference() const
{
    DEBUG("Checking if iterator was already dereferenced");
    if ( ! did_compute ) {
        DEBUG("Checking if workhorse has been constructed");
        if ( work_horse.get() == NULL ) {
            DEBUG("Constructing workhorse");
            work_horse.reset( new WorkHorse(engine) );
            DEBUG("Constructed workhorse");
        }
        DEBUG("Computing result");
        work_horse->compute(base);
        DEBUG("Computed result");
        did_compute = true;
    } else {
        DEBUG("Iterator has already been dereferenced");
    }
    return work_horse->resultStructure;
}

Engine::_iterator::_iterator( Engine& engine, Input::iterator base )
: base(base),
  engine(engine),
  did_compute(false)
{
}

Engine::_iterator::WorkHorse::WorkHorse( Engine& engine )
: engine(engine),
  config(engine.config),
  maximumLimit(20),
  flattener( *engine.imProp ),
  maximums(config.nms_x() / camera::pixel,
         config.nms_y() / camera::pixel,
         1, 1),
  origMotivation( config.motivation() )
{
    DEBUG("Started piston");
    DEBUG("Building spot finder with dimensions " << engine.imProp->size[0] <<
           " " << engine.imProp->size[1]);
    if ( ! config.spotFindingMethod.isValid() )
        throw std::runtime_error("No spot finding method selected.");
    if ( engine.imProp->plane_count() < 1 )
        throw std::runtime_error("Zero or less optical paths given for input, cannot compute.");
    if ( engine.imProp->fluorophores.size() < 1 )
        throw std::runtime_error("Zero or less fluorophores given for input, cannot compute.");

    spot_finder::Job job( config, *engine.imProp, 
        engine.imProp->plane(0), engine.imProp->fluorophores[0]);
    finder = config.spotFindingMethod().make(job);

    DEBUG("Building spot fitter with " << engine.imProp->fluorophores.size() << " fluorophores");
    for (unsigned int fluorophore = 0; fluorophore < engine.imProp->fluorophores.size(); ++fluorophore) {
        JobInfo info(config, *engine.imProp, fluorophore);
        fitter.push_back( config.spotFittingMethod().make(info) );
    }

    DEBUG("Building maximums");
    maximums.setLimit(maximumLimit);

    DEBUG("Initialized motivation");
    resultStructure.smoothed = &finder->getSmoothedImage();
    resultStructure.candidates = &maximums;
};

Engine::_iterator::_iterator( const _iterator& o ) 
: base( o.base ),
  engine(const_cast<Engine&>(o.engine)),
  did_compute(false)
{
}

void Engine::_iterator::WorkHorse::compute( Input::iterator base ) 
{
    buffer.clear();

    DEBUG("Intake (" << base->frame_number() << ")");

    Image& image = *base;
    if ( image.is_invalid() ) {
        resultStructure.forImage = base->frame_number();
        resultStructure.first = NULL;
        resultStructure.number = 0;
        resultStructure.source = image;

        ost::MutexLock lock( engine.mutex );
        engine.errors = engine.errors() + 1;
        engine.errors.viewable = true;
        return;
    } else {
        DEBUG("Image " << base->ptr() << " is valid");
    }

    base->background_standard_deviation() = background_variance * camera::ad_count;

    DEBUG("Compression (" << base->frame_number() << ")");
    IF_DSTORM_MEASURE_TIMES( clock_t prepre = clock() );
    const Image2D flattened = flattener.flatten_image( image );
    finder->smooth(flattened);
    IF_DSTORM_MEASURE_TIMES( smooth_time += clock() - prepre );

    CandidateTree<SmoothedPixel>::iterator cM = maximums.begin();
    int motivation;
    recompress:  /* We jump here if maximum limit proves too small */
    IF_DSTORM_MEASURE_TIMES( clock_t pre = clock() );
    finder->findCandidates( maximums );
    DEBUG("Found " << maximums.size() << " spots");

    IF_DSTORM_MEASURE_TIMES( clock_t search_start = clock() );
    IF_DSTORM_MEASURE_TIMES( search_time += search_start - pre );

    /* Motivational fitting */
    motivation = origMotivation;
    for ( cM = maximums.begin(); cM.hasMore() && motivation > 0; cM++){
        const Spot& s = cM->second;
        DEBUG("Trying candidate at " << s.x() << "," << s.y() << " at motivation " << motivation );
        /* Get the next spot to fit and fit it. */
        Localization *candidate = buffer.allocate(5), *start = candidate;
        double best_total_residues = std::numeric_limits<double>::infinity();
        int best_found = -1, fluorophore = 0;
        for (unsigned int fit_fluo = 0; fit_fluo < fitter.size(); ++fit_fluo) {
            candidate = start + std::max(0, best_found);
            int found_number = fitter[fit_fluo].fitSpot(s, image, candidate);
            double total_amplitude = 0, total_residues = 0;
            for (int j = 0; j < found_number; j++) {
                candidate[j].frame_number() = base->frame_number();
                total_amplitude += candidate[j].amplitude() / camera::ad_count;
            }
            if ( found_number > 0 )
                total_residues = candidate[0].fit_residues().value();
            else
                total_residues = std::numeric_limits<double>::infinity();
            DEBUG("Fitter " << fit_fluo << " found " << found_number << " with total amplitude " << total_amplitude << " and total residues " << total_residues);
            if ( total_residues < best_total_residues ) {
                for (int i = 0; i < best_found && i < found_number; ++i)
                    start[i] = candidate[found_number-i-1];
                best_found = found_number;
                fluorophore = fit_fluo;
                best_total_residues = total_residues;
            }
        }
        for (int i = 0; i < best_found; ++i)
            start[i].fluorophore = fluorophore;
        DEBUG("Committing " << best_found << " localizations found for fluorophore " << fluorophore << " at position " << start[0].position().transpose());
        buffer.commit(std::max(0,best_found));
        if ( best_found > 0 )
            motivation = origMotivation;
        else
            motivation += best_found;
    }
    if (motivation > 0 && cM.limitReached()) {
        maximumLimit *= 2;
        DEBUG("Raising maximumLimit to " << maximumLimit);
        maximums.setLimit(maximumLimit);
        buffer.clear();
        goto recompress;
    }

    DEBUG("Found " << buffer.size() << " localizations");
    IF_DSTORM_MEASURE_TIMES( fit_time += clock() - search_start );

    DEBUG("Power with " << buffer.size() << " localizations");
    resultStructure.forImage = base->frame_number();
    resultStructure.first = buffer.ptr();
    resultStructure.number = buffer.size();
    resultStructure.source = image;
}

Engine::_iterator::~_iterator() {
    DEBUG("Destructing iterator");
    work_horse.reset();
    DEBUG("Destructed iterator");
}

Engine::Base::iterator Engine::begin() {
    return Base::iterator( _iterator( *this, input->begin() ) );
}

Engine::Base::iterator Engine::end() {
    return Base::iterator( _iterator( *this, input->end() ) );
}

boost::ptr_vector<output::Output> Engine::additional_outputs()
{
    boost::ptr_vector<output::Output> rv;
    return rv;
}

void Engine::restart() { throw std::logic_error("Not implemented."); }
void Engine::stop() { throw std::logic_error("Not implemented."); }
void Engine::repeat_results() { throw std::logic_error("Not implemented."); }
bool Engine::can_repeat_results() { return false; }
void Engine::change_input_traits( std::auto_ptr< input::BaseTraits > traits )
{
    Input::TraitsPtr::element_type* t = dynamic_cast< Input::TraitsPtr::element_type* >(traits.get());
    if ( t != NULL ) {
        imProp.reset( t );
        traits.release();
    }
}

}
}
