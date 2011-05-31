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
    if ( p->flags.test( BaseSource::TimeCritical ) )
        new_source.reset( new Buffer<Type, true>(p) );
    else
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
    bool needs_to_be_buffered = true;
    if ( rv->flags.test( BaseSource::TimeCritical ) ) 
        needs_to_be_buffered = true;
    if ( ! rv->flags.test( BaseSource::Repeatable ) && my_config.needs_multiple_passes ) 
        needs_to_be_buffered = true;
    if ( ! rv->flags.test( BaseSource::MultipleConcurrentIterators ) 
         && my_config.needs_concurrent_iterators ) 
        needs_to_be_buffered = true;

    if ( needs_to_be_buffered )
    {
        DEBUG("Buffer is inserted into source chain");
        chain::DefaultVisitor<BufferConfig> visitor(my_config);
        return specialize_source( visitor, rv.release() );
    } else {
        DEBUG("Buffer is not needed, not inserting into source chain");
        return rv.release();
    }
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

chain::Link::AtEnd BufferChainLink::context_changed( ContextRef c, Link *link ) {
    Link::context_changed(c,link);
    if ( c.get() ) {
        my_config.needs_concurrent_iterators = c->need_multiple_concurrent_iterators;
        my_config.needs_multiple_passes = c->will_make_multiple_passes;
        my_config.throw_errors = c->throw_errors;
    }

    if ( my_config.needs_concurrent_iterators ) {
        chain::Context::Ptr no_concurrent( c->clone() );
        no_concurrent->need_multiple_concurrent_iterators = false;
        return Forwarder::notify_of_context_change(no_concurrent);
    } else {
        return Forwarder::notify_of_context_change(c);
    }
}

std::auto_ptr<chain::Forwarder> makeBufferChainLink() { 
    return std::auto_ptr<chain::Forwarder>( new BufferChainLink() );
}

}
}
