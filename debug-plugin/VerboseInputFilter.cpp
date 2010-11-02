#define VERBOSE
#include <boost/variant.hpp>
#include <boost/serialization/variant.hpp> 
#include "VerboseInputFilter.h"
#include <dStorm/input/chain/Context.h>
#include <dStorm/input/chain/MetaInfo.h>
#include <iostream>

VerboseInputFilter::AtEnd
VerboseInputFilter::traits_changed( TraitsRef ref, Link *link )
{
    Link::traits_changed( ref, link );
    if ( verbose() )
        DEBUG("Traits " << ref.get() << " are passing on " << getNode().getName() << " (" << this << ")");
    return notify_of_trait_change( ref );
}

VerboseInputFilter::AtEnd
VerboseInputFilter::context_changed( ContextRef ref, Link *link )
{
    Link::context_changed( ref, link );
    if ( verbose() )
        DEBUG("Context " << ref.get() << " is passing on " << getNode().getName() << " (" << this << ")");
    return notify_of_context_change( ref );
}

std::auto_ptr<dStorm::input::chain::Filter>
make_verbose_input_filter() {
    return std::auto_ptr<dStorm::input::chain::Filter>
        (new VerboseInputFilter());
}
