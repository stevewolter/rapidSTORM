#include "Singleton.h"

namespace dStorm {
namespace input {
namespace chain {

Singleton::Singleton() : Forwarder() {}

Singleton::~Singleton() {
}


BaseSource* Singleton::makeSource() {
    ost::MutexLock lock(mutex);
    return Forwarder::makeSource();
}

Singleton::AtEnd Singleton::traits_changed( TraitsRef r, Link* which )
{
    ost::MutexLock lock(mutex);
    meta_info = r;
    AtEnd rv;
    for ( Upstream::iterator i = less_specializeds.begin(); i != less_specializeds.end(); ++i )
        rv = (*i)->traits_changed( r, this );
    return rv;
}

Singleton::AtEnd Singleton::context_changed( ContextRef context, Link* link ) {
    ost::MutexLock lock(mutex);
    Link::context_changed( context, link );
    return Forwarder::notify_of_context_change( context );
}

simparm::Node& Singleton::getNode() {
    ost::MutexLock lock(mutex);
    return Forwarder::getNode();
}

void Singleton::set_upstream_element( Link& element, SetType type ) {
    ost::MutexLock lock(mutex);
    if ( type == Add ) {
        less_specializeds.insert( &element );
    } else {
        assert( less_specializeds.find(&element) != less_specializeds.end() );
        less_specializeds.erase( &element );
    }
}

void Singleton::remove_less_specialized(Link* link) {
    set_upstream_element(*link, Remove);
}

}
}
}
