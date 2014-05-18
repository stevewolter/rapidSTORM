#define DSTORM_ENGINE_CPP

#include "debug.h"

#include "engine/EngineDebug.h"
#include "engine/Engine.h"

#include <cassert>

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/thread/locks.hpp>

#include "input/Source.h"
#include "engine/FitPositionRoundRobin.h"
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
#include "engine/Config.h"
#include "simparm/dummy_ui/fwd.h"

namespace dStorm {
namespace engine {

class Engine::WorkHorse {
    Engine& engine;
    Config& config;
    Input::TraitsPtr meta_info;

    boost::ptr_vector<spot_fitter::Implementation> fitter;
    FitPositionRoundRobin position_generator;
    int origMotivation;

    bool compute_if_enough_positions(const ImageStack& image,
                                     output::LocalizedImage* target);

  public:
    WorkHorse( Engine& engine );
    ~WorkHorse() {}
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
  position_generator(engine.config, *meta_info),
  origMotivation( config.motivation() + meta_info->plane_count() - 1 )
{
    DEBUG("Started piston");
    if ( meta_info->plane_count() < 1 )
        throw std::runtime_error("Zero or less optical paths given for input, cannot compute.");
    if ( meta_info->fluorophore_count < 1 )
        throw std::runtime_error("Zero or less fluorophores given for input, cannot compute.");

    DEBUG("Building spot fitter with " << meta_info->fluorophore_count << " fluorophores");
    for (unsigned int fluorophore = 0; fluorophore < meta_info->fluorophore_count; ++fluorophore) {
        JobInfo info(meta_info, fluorophore, config.fit_judging_method() );
        fitter.push_back( config.spotFittingMethod().make(info) );
    }
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
    position_generator.compute_positions(image);

    while (!compute_if_enough_positions(image, target)) {
        position_generator.extend_range();
        target->clear();
    }

    DEBUG("Found " << target->size() << " localizations");
    target->forImage = image.frame_number();
    target->source = image;
}

bool Engine::WorkHorse::compute_if_enough_positions(
    const ImageStack& image, output::LocalizedImage* target) {
    /* Motivational fitting */
    int motivation = origMotivation;
    while (motivation > 0) {
        FitPosition fit_position;
        if (!position_generator.next_position(&fit_position)) {
            DEBUG("Not enough positions saved in position generator");
            return false;
        }
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
        }
    }

    return true;
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
