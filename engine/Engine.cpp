#define DSTORM_ENGINE_CPP

#include "debug.h"

#include "engine/EngineDebug.h"
#include "engine/Engine.h"

#include <cassert>

#include <boost/thread/locks.hpp>

#include "input/Source.h"
#include "output/Traits.h"
#include "output/LocalizedImage_traits.h"
#include "engine/JobInfo.h"
#include "image/constructors.h"
#include "image/slice.h"
#include "helpers/back_inserter.h"
#include "traits/Projection.h"
#include "engine/Config.h"
#include "simparm/dummy_ui/fwd.h"

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
    rvt->input_image_traits.reset( imProp.clone() );

    for (int fluorophore = 0; fluorophore < imProp.fluorophore_count; ++fluorophore) {
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
        work_horses.emplace_back(new EngineThread(*this, config, imProp));
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

void Engine::increment_error_count() {
    boost::lock_guard<boost::mutex> lock( mutex );
    errors = errors() + 1;
    errors.show();
}

}
}
