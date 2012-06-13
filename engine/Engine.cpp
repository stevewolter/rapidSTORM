#define DSTORM_ENGINE_CPP

#include "debug.h"

#include "EngineDebug.h"
#include "engine/Engine.h"

#include <cassert>
#include <iterator>

#include <dStorm/input/Source.h>
#include <dStorm/engine/SpotFinder.h>
#include <dStorm/engine/SpotFitterFactory.h>
#include <dStorm/engine/SpotFitter.h>
#include <dStorm/output/Traits.h>
#include <dStorm/output/LocalizedImage_traits.h>
#include <dStorm/output/LocalizedImage.h>
#include <dStorm/engine/JobInfo.h>
#include <dStorm/image/constructors.h>
#include <dStorm/image/slice.h>
#include <dStorm/helpers/back_inserter.h>
#include <dStorm/traits/Projection.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include "PlaneFlattener.h"
#include "Config.h"

#ifdef DSTORM_MEASURE_TIMES
#include <time.h>
clock_t smooth_time = 0, search_time = 0, fit_time = 0;
#endif

namespace dStorm {
namespace engine {

Engine::Engine(
    const Config &config, 
    std::auto_ptr<Input> input
)
: input(input), 
  config(config),
  errors("ErrorCount", "Number of dropped images", 0)
{
    DEBUG("Constructing engine");

    errors.freeze();
    errors.hide();

}

void Engine::attach_ui_( simparm::NodeHandle n ) {
    input->attach_ui( n );
    errors.attach_ui( n );
}

Engine::~Engine() {
    DEBUG("Destructing engine");
    input.reset();
    DEBUG("Destructed engine");
}

boost::shared_ptr< input::Traits<output::LocalizedImage> >
Engine::convert_traits( Config& config, const input::Traits<engine::ImageStack>& imProp )
{
    assert( imProp.plane_count() > 0 );
    input::Traits<Localization> rv;
    rv.in_sequence = true;
    rv.image_number() = imProp.image_number();

    DEBUG("Getting other traits dimensionality");
    DEBUG("Getting minimum amplitude");
    if ( ! config.guess_threshold() )
        rv.amplitude().range().first = config.amplitude_threshold();

    boost::shared_ptr< input::Traits<output::LocalizedImage> > rvt( 
        new TraitsPtr::element_type( rv, "Engine", "Localizations" ) );
    rvt->source_image_is_set = true;
    rvt->smoothed_image_is_set = true;
    rvt->candidate_tree_is_set = true;
    rvt->input_image_traits.reset( imProp.clone() );

    DEBUG("Setting traits from spot fitter");
    for (unsigned int fluorophore = 0; fluorophore < imProp.fluorophores.size(); ++fluorophore) {
        DEBUG("Constructing spot fitting info");
        JobInfo info(
            config.amplitude_threshold() ,
            imProp, fluorophore);
        DEBUG("Constructed spot fitting info at " << &info << ", setting traits with " << &config.spotFittingMethod() );
        config.spotFittingMethod().set_traits( *rvt, info );
        DEBUG("Finished setting traits, info now at " << &info);
    }
    DEBUG("Returning traits");

    rvt->fluorophore().is_given = imProp.fluorophores.size() > 1;
    rvt->fluorophore().range().first = 0;
    rvt->fluorophore().range().second = imProp.fluorophores.size() - 1;

    return rvt;
}

Engine::TraitsPtr Engine::get_traits(Wishes w) {
    if ( &config.spotFittingMethod() == NULL )
        throw std::runtime_error("No spot fitter selected");
    DEBUG("Retrieving input traits");
    if ( config.guess_threshold() )
        w.set( InputStandardDeviation );

    if ( imProp.get() == NULL )
        imProp = input->get_traits(w);
    DEBUG("Retrieved input traits");

    if ( config.guess_threshold() ) {
        DEBUG("Guessing input threshold");
        bool have_set_threshold = false;
        for ( int i = 0; i < imProp->plane_count(); ++i ) {
            if ( imProp->plane(i).optics.background_stddev.is_initialized() ) {
                if ( imProp->optics(i).transmission_coefficient(0) > 1E-2 ) {
                    camera_response threshold = 
                        config.threshold_height_factor() * *imProp->optics(i).background_stddev / 
                            imProp->optics(i).transmission_coefficient(0);
                    if ( ! have_set_threshold || config.amplitude_threshold() > threshold )
                        config.amplitude_threshold = threshold;
                    have_set_threshold = true;
                }
            }
        }
        if ( ! have_set_threshold )
            throw std::runtime_error("Amplitude threshold is not set and could not be determined from background noise strength");
        DEBUG("Guessed amplitude threshold " << config.amplitude_threshold());
    } else {
        DEBUG("Using amplitude threshold " << config.amplitude_threshold());
    }

    input::Traits<output::LocalizedImage>::Ptr prv =
        convert_traits(config, *imProp);

    std::pair<samplepos,samplepos> size = imProp->size_in_sample_space();
    for (int i = 0; i < 2; ++i) {
        prv->position().range()[i].first = size.first[i];
        prv->position().range()[i].second = size.second[i];
    }

    prv->carburettor = input.get();
    prv->image_number().is_given = true;
    prv->engine = this;

    this->config.spotFittingMethod().check_configuration( *imProp, *prv );

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
    Input::TraitsPtr meta_info;

    int maximumLimit;
    PlaneFlattener flattener;
    std::auto_ptr<spot_finder::Base> finder;
    boost::ptr_vector<spot_fitter::Implementation> fitter;
    CandidateTree<SmoothedPixel> maximums;
    int origMotivation;
    FitPosition get_fit_position( const Spot& ) const;

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

FitPosition Engine::_iterator::WorkHorse::get_fit_position( const Spot& spot ) const
{
    return
        meta_info->plane(0).projection().
            pixel_in_sample_space( spot.position() ).head<2>()
            .cast< FitPosition::Scalar >(); 
}


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
  meta_info( engine.imProp ),
  maximumLimit(20),
  flattener( *meta_info, engine.make_plane_weight_vector() ),
  maximums(config.nms().x() / camera::pixel, config.nms().y() / camera::pixel,
         1, 1),
  origMotivation( config.motivation() )
{
    DEBUG("Started piston");
    if ( ! config.spotFindingMethod.isValid() )
        throw std::runtime_error("No spot finding method selected.");
    if ( meta_info->plane_count() < 1 )
        throw std::runtime_error("Zero or less optical paths given for input, cannot compute.");
    if ( meta_info->fluorophores.size() < 1 )
        throw std::runtime_error("Zero or less fluorophores given for input, cannot compute.");

    spot_finder::Job job( meta_info->plane(0), meta_info->fluorophores[0]);
    finder = config.spotFindingMethod().make(job);

    DEBUG("Building spot fitter with " << meta_info->fluorophores.size() << " fluorophores");
    for (unsigned int fluorophore = 0; fluorophore < meta_info->fluorophores.size(); ++fluorophore) {
        JobInfo info(config.amplitude_threshold(), *meta_info, fluorophore);
        fitter.push_back( config.spotFittingMethod().make(info) );
    }

    DEBUG("Building maximums");
    maximums.setLimit(maximumLimit);

    resultStructure.smoothed = finder->getSmoothedImage();
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
    resultStructure.clear();

    DEBUG("Intake (" << base->frame_number() << ")");

    ImageStack& image = *base;
    if ( image.has_invalid_planes() ) {
        resultStructure.forImage = base->frame_number();
        resultStructure.clear();
        resultStructure.source = image;

        boost::lock_guard<boost::mutex> lock( engine.mutex );
        engine.errors = engine.errors() + 1;
        engine.errors.show();
        return;
    } else {
        DEBUG("Image " << base->frame_number() << " is valid");
    }

    DEBUG("Compression (" << base->frame_number() << ")");
    IF_DSTORM_MEASURE_TIMES( clock_t prepre = clock() );
    const Image2D flattened = flattener.flatten_image( image );
    finder->smooth(flattened);
    IF_DSTORM_MEASURE_TIMES( smooth_time += clock() - prepre );

    int motivation;
    recompress:  /* We jump here if maximum limit proves too small */
    IF_DSTORM_MEASURE_TIMES( clock_t pre = clock() );
    finder->findCandidates( maximums );
    DEBUG("Found spots");

    IF_DSTORM_MEASURE_TIMES( clock_t search_start = clock() );
    IF_DSTORM_MEASURE_TIMES( search_time += search_start - pre );

    /* Motivational fitting */
    motivation = origMotivation;
    for ( CandidateTree<SmoothedPixel>::const_iterator cM = maximums.begin(), cE = maximums.end(); cM != cE; ++cM){
        FitPosition fit_position = get_fit_position( cM->spot() );
        DEBUG("Trying candidate " << fit_position.transpose() << " at motivation " << motivation );
        /* Get the next spot to fit and fit it. */
        output::LocalizedImage& buffer = resultStructure;
        int candidate = buffer.size(), start = candidate;
        double best_total_residues = std::numeric_limits<double>::infinity();
        int best_found = -1;
        for (unsigned int fit_fluo = 0; fit_fluo < fitter.size(); ++fit_fluo) {
            candidate = buffer.size();
            int found_number = fitter[fit_fluo].fitSpot(fit_position, image, 
                spot_fitter::Implementation::iterator( boost::back_inserter( buffer ) ) );
            double total_residues = 0;

            if ( found_number > 0 )
                for ( size_t i = candidate; i < buffer.size(); ++i )
                    total_residues = buffer[i].fit_residues().value();
            else
                total_residues = std::numeric_limits<double>::infinity();
            DEBUG("Fitter " << fit_fluo << " found " << found_number << " with total residues " << total_residues);
            if ( total_residues < best_total_residues ) {
                for (int i = 0; i < best_found && i < found_number; ++i)
                    buffer[start+i] = buffer[candidate+found_number-i-1];
                best_found = found_number;
                best_total_residues = total_residues;
            }
        }
        buffer.resize(start+std::max<int>(0,best_found));
        for (int i = 0; i < best_found; ++i) {
            buffer[i+start].frame_number() = base->frame_number();
        }
        if ( best_found > 0 ) {
            DEBUG("Committing " << best_found << " localizations found at position " << buffer[start].position().transpose());
            motivation = origMotivation;
        } else {
            motivation += best_found;
            DEBUG("No localizations, decreased motivation by " << -best_found 
                  << " to " << motivation);
            if ( motivation <= 0 ) break;
        }
    }
    if (motivation > 0 && maximums.reached_size_limit()) {
        maximumLimit *= 2;
        DEBUG("Raising maximumLimit to " << maximumLimit);
        maximums.setLimit(maximumLimit);
        resultStructure.clear();
        goto recompress;
    }

    DEBUG("Found " << resultStructure.size() << " localizations");
    IF_DSTORM_MEASURE_TIMES( fit_time += clock() - search_start );

    DEBUG("Power with " << resultStructure.size() << " localizations");
    resultStructure.forImage = base->frame_number();
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

std::auto_ptr<EngineBlock> Engine::block() {
    throw std::logic_error("Not implemented.");
}

std::vector<float> Engine::make_plane_weight_vector() const {
    assert( int(config.spot_finder_weights.size()) >= imProp->plane_count() - 1 );
    std::vector<float> rv;
    for (int i = 1; i < imProp->plane_count(); ++i)
        rv.push_back( config.spot_finder_weights[i-1]() );
    return rv;
}

}
}
