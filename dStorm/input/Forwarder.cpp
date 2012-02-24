#include "debug.h"
#include "Forwarder.h"
#include <simparm/Node.hh>
#include <dStorm/input/Source.h>

namespace dStorm {
namespace input {

Forwarder::Forwarder() {}
Forwarder::Forwarder(const Forwarder& o) : Link(o)
{
    if ( o.more_specialized.get() )
        more_specialized.reset( o.more_specialized->clone() );
    if ( more_specialized.get() )
        connection = more_specialized->notify( 
            boost::bind( &Forwarder::traits_changed, this, _1, more_specialized.get() ) );
}

Forwarder::~Forwarder() { 
    DEBUG("Destructing " << this);
}


void Forwarder::registerNamedEntries(simparm::Node& n) {
    if ( more_specialized.get() ) 
        more_specialized->registerNamedEntries(n);
}

std::auto_ptr<BaseSource> Forwarder::upstream_source() { 
    assert( more_specialized.get() );
    return more_specialized->make_source();
}

BaseSource* Forwarder::makeSource() { 
    return upstream_source().release();
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
    DEBUG("Forwarder " << this << " got meta info update from " << l << " for " << ref.get() );
    update_current_meta_info(ref);
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
    if ( more_specialized.get() )
        link->insert_new_node( more_specialized, Anywhere );
    more_specialized = link;
    if ( more_specialized.get() ) {
        connection = more_specialized->notify( 
            boost::bind( &Forwarder::traits_changed, this, _1, more_specialized.get() ) );
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
