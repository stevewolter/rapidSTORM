#define DSTORM_ENGINE_CPP

#include "debug.h"

#include "noop_engine/Engine.h"

#include <cassert>

#include "input/Source.h"
#include "output/Traits.h"
#include "output/LocalizedImage_traits.h"
#include "output/LocalizedImage.h"

namespace dStorm {
namespace noop_engine {

Engine::Engine( std::auto_ptr<Input> input )
: input(input)
{
}

Engine::TraitsPtr Engine::get_traits(Wishes w) {
    Engine::TraitsPtr rv = convert_traits( *input->get_traits(w) );
    rv->carburettor = input.get();
    return rv;
}

Engine::TraitsPtr Engine::convert_traits( const Input::Traits& p ) {
    Base::TraitsPtr prv( new input::Traits<output::LocalizedImage>("Noop", "Dummy engine data") );
    prv->in_sequence = true;
    prv->image_number() = p.image_number();
    prv->input_image_traits.reset( p.clone() );

    return prv;
}

void Engine::dispatch(Messages m) {
    input->dispatch(m);
}

bool Engine::GetNext(int thread, output::LocalizedImage* target) {
    engine::ImageStack image;
    if (!input->GetNext(thread, &image)) {
        return false;
    }

    target->forImage = image.frame_number();
    target->clear();
    target->source = image; 
    target->candidates = NULL;
    return true;
}

}
}
