#define BOOST_DISABLE_ASSERTS
#include "impl.h"
#include <dStorm/input/chain/Filter_impl.h>

namespace dStorm {
namespace input {
namespace chain {

template <>
template <>
bool
DefaultVisitor<locprec::biplane_alignment::Config>::operator()
    ( std::auto_ptr< dStorm::input::Source<dStorm::engine::Image> > rv )
{
    if ( config.model().is_identity() ) {
        new_source = rv;
    } else {
        DEBUG("Creating new filter");
        std::auto_ptr< dStorm::input::Source<dStorm::engine::Image> > my_filter( new locprec::biplane_alignment::Source(config, rv) );
        new_source = my_filter;
        DEBUG("Created new filter");
    }
    return true;
}

}
}
}

namespace locprec {
namespace biplane_alignment {

Filter::AtEnd
Filter::traits_changed( TraitsRef ref, Link *link )
{
    Link::traits_changed( ref, link );
    return notify_of_trait_change( ref );
}

Filter::AtEnd
Filter::context_changed( ContextRef ref, Link *link )
{
    Link::context_changed( ref, link );
    boost::shared_ptr< dStorm::input::chain::Context > mine( ref->clone() );
    mine->will_make_multiple_passes = true;
    return notify_of_context_change( mine );
}

dStorm::input::BaseSource* Filter::makeSource()
{
    return dStorm::input::chain::DelegateToVisitor::makeSource(*this);
}

std::auto_ptr<dStorm::input::chain::Filter>
make_filter() {
    return std::auto_ptr<dStorm::input::chain::Filter>(new Filter());
}

}
}
