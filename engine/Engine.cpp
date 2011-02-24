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

#include "SigmaGuesser.h"

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
    push_back( config.sigma_x);
    push_back( config.sigma_y);
    push_back( config.sigma_xy);
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

    boost::shared_ptr< input::Traits<output::LocalizedImage> > rvt( new TraitsPtr::element_type( rv ) );
    rvt->source_image_is_set = true;
    rvt->smoothed_image_is_set = true;
    rvt->candidate_tree_is_set = true;
    rvt->input_image_traits.reset( imProp->clone() );

    return rvt;
}

Engine::TraitsPtr Engine::get_traits() {
    imProp = input->get_traits();

    if ( ! config.amplitude_threshold().is_set() ) {
        DEBUG("Guessing input threshold");
        if ( imProp->background_stddev.is_set() ) {
            config.amplitude_threshold = 35.0f * (*imProp->background_stddev);
        } else {
            throw std::logic_error("Amplitude threshold is not set and could not be determined from background noise strength");
        }
        DEBUG("Guessed amplitude threshold " << *config.amplitude_threshold());
    } else {
        DEBUG("Using amplitude threshold " << *config.amplitude_threshold());
    }

    input::Traits<output::LocalizedImage>::Ptr prv =
        convert_traits(config, imProp);
    prv->carburettor = input.get();
    prv->image_number().is_given = true;

    DEBUG("Setting traits from spot fitter");
    JobInfo info(config, *imProp);
    config.spotFittingMethod().set_traits( *prv, info );
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
    boost::ptr_vector<SpotFinder> finder;
    std::auto_ptr<SpotFitter> fitter;
    data_cpp::Vector<Localization> buffer;
    CandidateTree<SmoothedPixel> maximums;
    int origMotivation;

  public:
    WorkHorse( Engine& engine );
    ~WorkHorse() {
        DEBUG("Destructing spot finders");
        finder.clear();
        DEBUG("Destructing spot fitter");
        fitter.reset();
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
  maximums(config.x_maskSize() / camera::pixel,
         config.y_maskSize() / camera::pixel,
         1, 1),
  origMotivation( config.motivation() )
{
    DEBUG("Started piston");
    DEBUG("Building spot finders with dimensions " << engine.imProp->size[0] <<
           " " << engine.imProp->size[1]);
    if ( ! config.spotFindingMethod.isValid() )
        throw std::runtime_error("No spot finding method selected.");
    for (int i = 0; i < engine.imProp->size[2].value(); ++i)
        finder.push_back( config.spotFindingMethod().make_SpotFinder(config, engine.imProp->size) );

    DEBUG("Building spot fitter");
    fitter = config.spotFittingMethod().make_by_parts(config, *engine.imProp);

    DEBUG("Building maximums");
    maximums.setLimit(maximumLimit);

    DEBUG("Initialized motivation");
    resultStructure.smoothed = &finder[0].getSmoothedImage();
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
    for (int i = 0; i < image.depth_in_pixels(); ++i)
        finder[i].smooth(image.slice(2,i * camera::pixel));
    IF_DSTORM_MEASURE_TIMES( smooth_time += clock() - prepre );

    CandidateTree<SmoothedPixel>::iterator cM = maximums.begin();
    int motivation;
    recompress:  /* We jump here if maximum limit proves too small */
    IF_DSTORM_MEASURE_TIMES( clock_t pre = clock() );
    for (int i = 0; i < image.depth_in_pixels(); ++i)
        finder[i].findCandidates( maximums );
    DEBUG("Found " << maximums.size() << " spots");

    IF_DSTORM_MEASURE_TIMES( clock_t search_start = clock() );
    IF_DSTORM_MEASURE_TIMES( search_time += search_start - pre );

    /* Motivational fitting */
    motivation = origMotivation;
    for ( cM = maximums.begin(); cM.hasMore() && motivation > 0; cM++){
        const Spot& s = cM->second;
        DEBUG("Trying candidate at " << s.x() << "," << s.y() );
        /* Get the next spot to fit and fit it. */
        Localization *candidate = buffer.allocate();
        int found_number = fitter->fitSpot(s, image, candidate);
        if ( found_number > 0 ) {
            DEBUG("Good fit");
            for (int j = 0; j < found_number; j++)
                candidate[j].frame_number() = base->frame_number();
            motivation = origMotivation;
            buffer.commit(found_number);
        } else if ( found_number < 0 ) {
            DEBUG("Bad fit");
            motivation += found_number;
        }
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

    DEBUG("Making SigmaGuesser");
    if ( ! config.fixSigma() ) 
        rv.push_back( new SigmaGuesserMean( config ) );
    return rv;
}

}
}
