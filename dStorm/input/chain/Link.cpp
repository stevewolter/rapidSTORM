#include "debug.h"
#include "Link.h"
#include <simparm/Node.hh>

#ifdef VERBOSE
#include "dStorm/engine/Image.h"
#include "dStorm/ImageTraits.h"
#include "MetaInfo.h"
#endif

namespace dStorm {
namespace input {
namespace chain {

Link::Link() 
: less_specialized(NULL)
{
}

Link::Link(const Link& o) 
: less_specialized(NULL), meta_info(o.meta_info)
{
    DEBUG("Cloned " << o.getNode().getName() << " to have context " << meta_info.get());
}

Link::~Link() {
}

Link::AtEnd Link::notify_of_trait_change( TraitsRef new_traits ) 
{
    DEBUG(this << " got trait change to " << new_traits.get());
    meta_info = new_traits;
    if ( less_specialized )  {
        return less_specialized->traits_changed( new_traits, this );
    } else {
        return AtEnd();
    }
}

void Link::set_upstream_element( Link& element, SetType type ) {
    if ( type == Add ) {
        assert( less_specialized == NULL );
        less_specialized = &element;
    } else {
        assert( less_specialized == &element );
        less_specialized = NULL;
    }
}

Link::AtEnd Link::traits_changed( TraitsRef r, Link* l ) {
    DEBUG("Traits " << r.get() << " providing " << ((r.get() && r->provides_nothing()) ? "nothing" : "something") << " passing from " << l << "(" << ((l) ? l->getNode().getName() : "NULL") << ") by " << this << "(" << getNode().getName()  << ")");
    return AtEnd();
}

Terminus::AtEnd Terminus::traits_changed( TraitsRef, Link* )
{
    /* This point should never be reached since a chain terminus cannot
     * be a less specialized element. */
    assert( false );
    return AtEnd();
}

void Terminus::insert_new_node( std::auto_ptr<Link> l, Place ) {
        throw std::logic_error("No insertion point found for " + l->description());
    }

std::string Link::name() const { throw std::logic_error("Not implemented"); }

}
}
}

namespace boost {
template <>
dStorm::input::chain::Link* new_clone<dStorm::input::chain::Link>
    ( const dStorm::input::chain::Link& l )
{ return l.clone(); }
template <>
void delete_clone<dStorm::input::chain::Link>(const dStorm::input::chain::Link* l)
{
    delete l;
}
}
