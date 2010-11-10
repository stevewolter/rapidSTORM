#define VERBOSE
#include <boost/variant.hpp>
#include <boost/serialization/variant.hpp> 
#include "VerboseInputFilter.h"
#include <dStorm/input/chain/Context.h>
#include <dStorm/input/chain/MetaInfo.h>
#include <dStorm/input/chain/Filter_impl.h>
#include <iostream>

namespace dStorm {
namespace input {
namespace chain {

template <>
template <typename Type>
bool
DefaultVisitor<VerboseInputFilter::Config>::operator()
    ( std::auto_ptr< dStorm::input::Source<Type> > rv )
{
    if ( config.verbose() ) {
        DEBUG( "Source of type " << typeid(*rv.get()).name() << " is passing" );
        new_source.reset( new VerboseInputFilter::Source<Type>(config, rv) );
    } else
        new_source.reset( rv.release() );
    return true;
}

}
}
}

namespace VerboseInputFilter {

VerboseInputFilter::AtEnd
VerboseInputFilter::traits_changed( TraitsRef ref, Link *link )
{
    Link::traits_changed( ref, link );
    if ( config.verbose() )
        DEBUG("Traits " << ref.get() << " are passing on " << getNode().getName() << " (" << this << ")");
    return notify_of_trait_change( ref );
}

VerboseInputFilter::AtEnd
VerboseInputFilter::context_changed( ContextRef ref, Link *link )
{
    Link::context_changed( ref, link );
    if ( config.verbose() )
        DEBUG("Context " << ref.get() << " is passing on " << getNode().getName() << " (" << this << ")");
    return notify_of_context_change( ref );
}

dStorm::input::BaseSource* VerboseInputFilter::makeSource()
{
    return dStorm::input::chain::DelegateToVisitor::makeSource(*this);
}

}

std::auto_ptr<dStorm::input::chain::Filter>
make_verbose_input_filter() {
    return std::auto_ptr<dStorm::input::chain::Filter>
        (new VerboseInputFilter::VerboseInputFilter());
}
