#include "debug.h"
#include "Forwarder.h"
#include <simparm/Node.hh>
#include <dStorm/input/Source.h>

namespace dStorm {
namespace input {
namespace chain {

Forwarder::Forwarder() {}
Forwarder::Forwarder(const Forwarder& o) : Link(o)
{
    if ( o.more_specialized.get() ) {
        more_specialized.reset( o.more_specialized->clone() );
    }
    if ( more_specialized.get() ) {
        set_upstream_element( *more_specialized, *this, Add ); 
    }
}

Forwarder::~Forwarder() { 
}


void Forwarder::registerNamedEntries(simparm::Node& n) {
    if ( more_specialized.get() ) 
        more_specialized->registerNamedEntries(n);
}

BaseSource* Forwarder::makeSource() { 
    assert( more_specialized.get() );
    BaseSource* ms = more_specialized->makeSource();
    assert( ms );
    return ms;
}

void Forwarder::insert_new_node( std::auto_ptr<Link> n, Place p ) {
    if ( more_specialized.get() )
        more_specialized->insert_new_node(n,p);
    else
        insert_here( n );
}

Link::TraitsRef Forwarder::upstream_traits() const {
    assert( more_specialized.get() );
    return more_specialized->current_meta_info();
}

void Forwarder::traits_changed( TraitsRef ref, Link* l ) {
    Link::traits_changed(ref,l);
    return update_current_meta_info(ref);
}

std::string Forwarder::name() const { 
    assert( more_specialized.get() );
    return more_specialized->name();
}

std::string Forwarder::description() const {
    assert( more_specialized.get() );
    return more_specialized->description();
}

void Forwarder::insert_here( std::auto_ptr<Link> link ) {
    if ( more_specialized.get() ) {
        set_upstream_element( *more_specialized, *this, Remove ); 
        link->insert_new_node( more_specialized, Anywhere );
    }
    more_specialized = link;
    if ( more_specialized.get() ) {
        set_upstream_element( *more_specialized, *this, Add ); 
    }
}

void Forwarder::publish_meta_info() {
    if ( ! more_specialized.get() )
        throw std::logic_error(name() + " needs a subinput to publish meta info");
    DEBUG(name() << " calls for publishment of meta info");
    more_specialized->publish_meta_info();
    DEBUG(name() << " called for publishment of meta info");
    if ( ! current_meta_info().get() )
        throw std::logic_error(name() + " did not publish meta info on request");
}

}
}
}
