#include "VerboseInputFilter.h"
#include <dStorm/input/chain/Context.h>
#include <dStorm/input/chain/MetaInfo.h>
#include <iostream>

VerboseInputFilter::AtEnd
VerboseInputFilter::traits_changed( TraitsRef ref, Link *link )
{
    Link::traits_changed( ref, link );
    std::cerr << "Traits " << ref.get() << " are passing on " << this << std::endl;
    return notify_of_trait_change( ref );
}

VerboseInputFilter::AtEnd
VerboseInputFilter::context_changed( ContextRef ref, Link *link )
{
    Link::context_changed( ref, link );
    std::cerr << "Context " << ref.get() << " is passing on " << this << std::endl;
    return notify_of_context_change( ref );
}
