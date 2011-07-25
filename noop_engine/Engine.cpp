#define DSTORM_ENGINE_CPP

#include "debug.h"

#include "Engine.h"

#include <cassert>

#include <dStorm/input/Source_impl.h>
#include <dStorm/output/Traits.h>
#include <dStorm/output/LocalizedImage_traits.h>
#include <dStorm/output/LocalizedImage.h>

namespace dStorm {
namespace noop_engine {

Engine::Engine( std::auto_ptr<Input> input )
: Base(*this, input->flags ),
  Object("EngineStatus", "Computation status"),
  input(input)
{
    push_back( *this->input );
}

Engine::TraitsPtr Engine::get_traits() {
    Engine::TraitsPtr rv = convert_traits( *input->get_traits() );
    rv->carburettor = input.get();
    return rv;
}

Engine::TraitsPtr Engine::convert_traits( const Input::Traits& p ) {
    Base::TraitsPtr prv( new input::Traits<output::LocalizedImage>(p, "Noop", "Dummy engine data") );

    prv->source_image_is_set = true;

    return prv;
}

void Engine::dispatch(Messages m) {
    input->dispatch(m);
}

class Engine::_iterator
: public boost::iterator_facade< 
    _iterator, 
    output::LocalizedImage,
    std::input_iterator_tag>
{
    Input::iterator base;
    mutable output::LocalizedImage resultStructure;
    bool created;
    void create() {
        resultStructure.forImage = base->frame_number();
        resultStructure.clear();
        resultStructure.source = *base; 
        resultStructure.smoothed = NULL;
        resultStructure.candidates = NULL;
    }

    output::LocalizedImage& dereference() const {
        if ( !created ) const_cast<_iterator&>(*this).create();
        return resultStructure; 
    }
    bool equal(const _iterator& o) const { return base == o.base; }
    void increment() { ++base; created = false; }
    friend class boost::iterator_core_access;

  public:
    _iterator( Input::iterator base ) : base(base) { created = false; }
};

Engine::Base::iterator Engine::begin() {
    return Base::iterator( _iterator( input->begin() ) );
}

Engine::Base::iterator Engine::end() {
    return Base::iterator( _iterator( input->end() ) );
}

}
}
