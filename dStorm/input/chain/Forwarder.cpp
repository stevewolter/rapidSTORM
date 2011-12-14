#include "Forwarder.h"
#include <simparm/Node.hh>
#include <dStorm/input/Source.h>

namespace dStorm {
namespace input {
namespace chain {

Forwarder::Forwarder()
: more_specialized( NULL ) {}
Forwarder::Forwarder(const Forwarder& o)
: Link(o), more_specialized( NULL ) {}
Forwarder::Forwarder(Link& more_specialized)
: more_specialized(&more_specialized) 
{ 
    set_upstream_element( more_specialized, *this, Add ); 
}

void Forwarder::set_more_specialized_link_element(Link* l) {
    if ( more_specialized ) {
        set_upstream_element( *more_specialized, *this, Remove ); 
    }
    more_specialized = (l) ;
    if ( l ) 
        set_upstream_element( *more_specialized, *this, Add ); 
}

simparm::Node& Forwarder::getNode() {
    assert( more_specialized );
    return more_specialized->getNode();
}

Link::AtEnd Forwarder::notify_of_context_change( ContextRef new_context )
{
    assert( more_specialized );
    return more_specialized->context_changed(new_context, this);
}

Forwarder::~Forwarder() { 
}

BaseSource* Forwarder::makeSource() { 
    assert( more_specialized );
    BaseSource* ms = more_specialized->makeSource();
    assert( ms );
    return ms;
}

void Forwarder::insert_new_node( std::auto_ptr<Link> n, Place p ) {
    assert( more_specialized );
    more_specialized->insert_new_node(n,p);
}

}
}
}
