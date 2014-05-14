#define DSTORM_ENGINE_CPP

#include "debug.h"

#include "engine/EngineDebug.h"
#include "engine/Engine.h"

#include <cassert>

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/thread/locks.hpp>

#include "input/Source.h"
#include "engine/SpotFinder.h"
#include "engine/SpotFitterFactory.h"
#include "engine/SpotFitter.h"
#include "output/Traits.h"
#include "output/LocalizedImage_traits.h"
#include "output/LocalizedImage.h"
#include "engine/JobInfo.h"
#include "image/constructors.h"
#include "image/slice.h"
#include "helpers/back_inserter.h"
#include "traits/Projection.h"
#include "engine/PlaneFlattener.h"
#include "engine/Config.h"
#include <simparm/dummy_ui/fwd.h>

#ifdef DSTORM_MEASURE_TIMES
#include <time.h>
clock_t smooth_time = 0, search_time = 0, fit_time = 0;
#endif

namespace dStorm {
namespace engine {

class Engine::WorkHorse {
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
    void compute(const ImageStack& image, output::LocalizedImage* target);

    output::LocalizedImage resultStructure;
};


Engine::Engine(
    const Config &config, 
    std::auto_ptr<Input> input
)
: input(input), 
  config(config),
  errors("ErrorCount", "Number of dropped images", 0)
{
    DEBUG("Constructing engine");

    this->config.attach_ui( simparm::dummy_ui::make_node() );
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

    boost::shared_ptr< input::Traits<output::LocalizedImage> > rvt( 
        new TraitsPtr::element_type( rv, "Engine", "Localizations" ) );
    rvt->source_image_is_set = true;
    rvt->smoothed_image_is_set = true;
    rvt->candidate_tree_is_set = true;
    rvt->input_image_traits.reset( imProp.clone() );

    for (unsigned int fluorophore = 0; fluorophore < imProp.fluorophore_count; ++fluorophore) {
        JobInfo info( rvt->input_image_traits, fluorophore, config.fit_judging_method() );
        config.spotFittingMethod().set_traits( *rvt, info );
    }

    rvt->fluorophore().is_given = imProp.fluorophore_count > 1;
    rvt->fluorophore().range().first = 0;
    rvt->fluorophore().range().second = imProp.fluorophore_count - 1;

    return rvt;
}

Engine::TraitsPtr Engine::get_traits() {
    if ( &config.spotFittingMethod() == NULL )
        throw std::runtime_error("No spot fitter selected");

    if ( imProp.get() == NULL )
        imProp = input->get_traits();
    DEBUG("Retrieved input traits");

    input::Traits<output::LocalizedImage>::Ptr prv =
        convert_traits(config, *imProp);

    std::pair<samplepos,samplepos> size = imProp->size_in_sample_space();
    prv->position_x().range().first = size.first.x();
    prv->position_y().range().first = size.first.y();
    prv->position_x().range().second = size.second.x();
    prv->position_y().range().second = size.second.y();

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

void Engine::set_thread_count(int num_threads) {
    while (int(work_horses.size()) < num_threads) {
        work_horses.emplace_back(new WorkHorse(*this));
    }
}

FitPosition Engine::WorkHorse::get_fit_position( const Spot& spot ) const
{
    return
        boost::units::value(meta_info->plane(0).projection().
            pixel_in_sample_space( spot.position() ).head<2>()).cast<double>() * 1E6;
}

bool Engine::GetNext(int thread, output::LocalizedImage* target) {
    ImageStack image;
    if (!input->GetNext(thread, &image)) {
        return false;
    }

    DEBUG("Pushing image " << image.frame_number() << " into engine");
    work_horses[thread]->compute(image, target);
    return true;
}

Engine::WorkHorse::WorkHorse( Engine& engine )
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
    if ( meta_info->fluorophore_count < 1 )
        throw std::runtime_error("Zero or less fluorophores given for input, cannot compute.");

    spot_finder::Job job( meta_info->plane(0));
    finder = config.spotFindingMethod().make(job);

    DEBUG("Building spot fitter with " << meta_info->fluorophore_count << " fluorophores");
    for (unsigned int fluorophore = 0; fluorophore < meta_info->fluorophore_count; ++fluorophore) {
        JobInfo info(meta_info, fluorophore, config.fit_judging_method() );
        fitter.push_back( config.spotFittingMethod().make(info) );
    }

    DEBUG("Building maximums");
    maximums.setLimit(maximumLimit);

    resultStructure.smoothed = finder->getSmoothedImage();
    resultStructure.candidates = &maximums;
};

void Engine::WorkHorse::compute( const ImageStack& image, output::LocalizedImage* target ) 
{
    target->clear();

    DEBUG("Intake (" << image.frame_number() << ")");

    if ( image.has_invalid_planes() ) {
        target->forImage = image.frame_number();
        target->source = image;

        boost::lock_guard<boost::mutex> lock( engine.mutex );
        engine.errors = engine.errors() + 1;
        engine.errors.show();
        return;
    } else {
        DEBUG("Image " << image.frame_number() << " is valid");
    }

    DEBUG("Compression (" << image.frame_number() << ")");
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
        int candidate = target->size(), start = candidate;
        double best_total_residues = std::numeric_limits<double>::infinity();
        int best_found = -1;
        for (unsigned int fit_fluo = 0; fit_fluo < fitter.size(); ++fit_fluo) {
            candidate = target->size();
            int found_number = fitter[fit_fluo].fitSpot(fit_position, image, 
                spot_fitter::Implementation::iterator( boost::back_inserter( *target ) ) );
            double total_residues = 0;

            if ( found_number > 0 )
                for ( size_t i = candidate; i < target->size(); ++i )
                    total_residues = (*target)[i].fit_residues().value();
            else
                total_residues = std::numeric_limits<double>::infinity();
            DEBUG("Fitter " << fit_fluo << " found " << found_number << " with total residues " << total_residues);
            if ( total_residues < best_total_residues ) {
                for (int i = 0; i < best_found && i < found_number; ++i)
                    (*target)[start+i] = (*target)[candidate+found_number-i-1];
                best_found = found_number;
                best_total_residues = total_residues;
            }
        }
        target->resize(start+std::max<int>(0,best_found));
        for (int i = 0; i < best_found; ++i) {
            (*target)[i+start].frame_number() = image.frame_number();
        }
        if ( best_found > 0 ) {
            DEBUG("Committing " << best_found << " localizations found at position " << (*target)[start].position().transpose());
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
        target->clear();
        goto recompress;
    }

    DEBUG("Found " << target->size() << " localizations");
    IF_DSTORM_MEASURE_TIMES( fit_time += clock() - search_start );

    DEBUG("Power with " << target->size() << " localizations");
    target->forImage = image.frame_number();
    target->source = image;
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
