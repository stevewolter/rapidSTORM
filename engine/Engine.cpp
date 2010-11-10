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
    input::Traits<Localization> rv(imProp->get_other_dimensionality<Localization::Dim>());
    DEBUG("Getting other traits dimensionality");
    DEBUG("Getting minimum amplitude");
    rv.min_amplitude = config.amplitude_threshold();
    rv.speed = imProp->speed;
    rv.first_frame = imProp->first_frame;
    rv.last_frame = imProp->last_frame;
    DEBUG("Last frame is set in input: " << imProp->last_frame.is_set());
    DEBUG("Last frame is set: " << rv.last_frame.is_set());

    boost::shared_ptr< input::Traits<output::LocalizedImage> > rvt( new TraitsPtr::element_type( rv ) );
    rvt->source_image_is_set = true;
    rvt->smoothed_image_is_set = true;
    rvt->candidate_tree_is_set = true;

    return rvt;
}

Engine::TraitsPtr Engine::get_traits() {
    imProp = input->get_traits();

    if ( ! config.amplitude_threshold().is_set() ) {
        DEBUG("Guessing input threshold");
        if ( imProp->background_stddev.is_set() ) {
            config.amplitude_threshold = 35.0f * (*imProp->background_stddev);
        } else {
            throw std::logic_error("Background standard deviation is neither set nor could be computed");
        }
        DEBUG("Guessed amplitude threshold " << *config.amplitude_threshold());
    } else {
        DEBUG("Using amplitude threshold " << *config.amplitude_threshold());
    }

    input::Traits<output::LocalizedImage>::Ptr prv =
        convert_traits(config, imProp);
    prv->carburettor = input.get();

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
    Config& config;
    int maximumLimit;
    std::auto_ptr<SpotFinder> finder;
    std::auto_ptr<SpotFitter> fitter;
    data_cpp::Vector<Localization> buffer;
    CandidateTree<SmoothedPixel> maximums;
    int origMotivation;

    output::LocalizedImage resultStructure;
    bool did_compute;

    void compute();
    void init();

    /*=== iterator_facade interface ===*/
    output::LocalizedImage& dereference() const {
        if ( ! did_compute ) {
            const_cast<_iterator&>(*this).compute();
        }
        return const_cast<output::LocalizedImage&>(resultStructure);
    }
    bool equal(const _iterator& o) const { return base == o.base; }
    void increment() { did_compute = false; ++base; }

  public:
    _iterator( Engine& engine, Input::iterator base );
    _iterator( const _iterator& );

    friend class boost::iterator_core_access;
};

Engine::_iterator::_iterator( Engine& engine, Input::iterator base )
: base(base),
  engine(engine),
  config(engine.config),
  maximumLimit(20),
  maximums(config.x_maskSize() / cs_units::camera::pixel,
         config.y_maskSize() / cs_units::camera::pixel,
         1, 1),
  origMotivation( engine.config.motivation() ),
  did_compute(false)
{
    init();
}

void Engine::_iterator::init() {
    if ( base == engine.input->end() ) return;

    DEBUG("Started piston");
    DEBUG("Building spot finder with dimensions " << engine.imProp->dimx() <<
           " " << engine.imProp->dimy());
    if ( ! config.spotFindingMethod.isValid() )
        throw std::runtime_error("No spot finding method selected.");
    finder = config.spotFindingMethod().make_SpotFinder(config, engine.imProp->size);

    DEBUG("Building spot fitter");
    fitter = config.spotFittingMethod().make_by_parts(config, *engine.imProp);

    DEBUG("Building maximums");
    maximums.setLimit(maximumLimit);

    DEBUG("Initialized motivation");
    resultStructure.smoothed = &finder->getSmoothedImage();
    resultStructure.candidates = &maximums;
};

Engine::_iterator::_iterator( const _iterator& o ) 
: base( o.base ),
  engine(const_cast<Engine&>(o.engine)),
  config(const_cast<Config&>(o.config)),
  maximumLimit(o.maximumLimit),
  maximums(config.x_maskSize() / cs_units::camera::pixel, 
           config.y_maskSize() / cs_units::camera::pixel, 1, 1),
  origMotivation(o.origMotivation),
  resultStructure(o.resultStructure),
  did_compute(o.did_compute)
{
    init();
}

void Engine::_iterator::compute() 
{
    buffer.clear();

    DEBUG("Intake (" << base->frame_number() << ")");

    Image& image = *base;
    if ( image.is_invalid() ) {
        resultStructure.forImage = base->frame_number();
        resultStructure.first = NULL;
        resultStructure.number = 0;
        resultStructure.source = &image;
        did_compute = true;

        ost::MutexLock lock( engine.mutex );
        engine.errors = engine.errors() + 1;
        engine.errors.viewable = true;
        return;
    } else {
        DEBUG("Image " << base->ptr() << " is valid");
    }

    base->background_standard_deviation() = background_variance * cs_units::camera::ad_count;

    DEBUG("Compression (" << base->frame_number() << ")");
    IF_DSTORM_MEASURE_TIMES( clock_t prepre = clock() );
    finder->smooth(image);
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
        DEBUG("Trying candidate at " << s.x() << "," << s.y() );
        /* Get the next spot to fit and fit it. */
        Localization *candidate = buffer.allocate();
        int found_number = fitter->fitSpot(s, image, candidate);
        if ( found_number > 0 ) {
            DEBUG("Good fit");
            for (int j = 0; j < found_number; j++)
                candidate[j].setImageNumber( base->frame_number() );
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
    resultStructure.source = &image;

    did_compute = true;
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
