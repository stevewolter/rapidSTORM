#include "debug.h"
#include "Buffer_impl.h"
#include <dStorm/engine/Image.h>
#include <dStorm/Localization.h>
#include <dStorm/input/chain/Context.h>
#include <dStorm/input/chain/Filter_impl.h>
#include <dStorm/input/chain/DefaultFilterTypes.h>

namespace dStorm { 
namespace input { 

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

template <>
bool DefaultVisitor<BufferConfig>::unknown_trait(std::string desc) const
{
    if ( config.throw_errors )
        throw std::runtime_error("Input data need buffering, but the standard dSTORM buffer is not "
                                 "configured for data of type " + desc);
    else
        return true;
}

}

input::BaseSource*
BufferChainLink::makeSource()
{
    std::auto_ptr<BaseSource> rv( Forwarder::makeSource() );
    assert( rv.get() );
    chain::DefaultVisitor<BufferConfig> visitor(my_config);
    return specialize_source( visitor, rv.release() );
}

BufferChainLink::BufferChainLink() 
: config("Buffer", "Buffer") 
{
    my_config.throw_errors = false;
}

chain::Link::AtEnd
BufferChainLink::traits_changed( TraitsRef r, Link* l ) {
    Link::traits_changed(r,l);
    chain::DefaultVisitor<BufferConfig> visitor(my_config);
    visit_traits( visitor, r );
    return notify_of_trait_change( r );
}

std::auto_ptr<chain::Forwarder> makeBufferChainLink() { 
    return std::auto_ptr<chain::Forwarder>( new BufferChainLink() );
}

}
}
