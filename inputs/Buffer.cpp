#include "debug.h"
#include "Buffer_impl.h"
#include <dStorm/engine/Image.h>
#include <dStorm/Localization.h>
#include <dStorm/input/chain/Context.h>
#include <dStorm/input/chain/Filter_impl.h>
#include <dStorm/input/chain/DefaultFilterTypes.h>

namespace dStorm { 
namespace input { 

class BufferConfig {
    typedef chain::DefaultTypes SupportedTypes;
};


namespace chain {

template <>
template <typename Type>
bool DefaultVisitor<BufferConfig>::operator()( input::Traits<Type>& ) {
    /* This method doesn't need to *do* anything, just confirm that the trait specialization exists. */
    return true;
}

template <>
template <typename Type>
bool DefaultVisitor<BufferConfig>::operator()( std::auto_ptr< Source<Type> > p ) {
    new_source.reset( new Buffer<Type, false>(p) );
    return true;
}

}

input::BaseSource*
BufferChainLink::makeSource()
{
    std::auto_ptr<BaseSource> rv( Forwarder::makeSource() );
    assert( rv.get() );
    BufferConfig c;
    chain::DefaultVisitor<BufferConfig> visitor(c);
    return specialize_source( visitor, rv.release() );
}

BufferChainLink::BufferChainLink() 
: config("Buffer", "Buffer") 
{
}

chain::Link::AtEnd
BufferChainLink::traits_changed( TraitsRef r, Link* l ) {
    Link::traits_changed(r,l);
    BufferConfig c;
    chain::DefaultVisitor<BufferConfig> visitor(c);
    visit_traits( visitor, r );
    return notify_of_trait_change( r );
}

std::auto_ptr<chain::Forwarder> makeBufferChainLink() { 
    return std::auto_ptr<chain::Forwarder>( new BufferChainLink() );
}

}
}
