#ifndef DSTORM_INPUT_FORWARDER_HPP
#define DSTORM_INPUT_FORWARDER_HPP

#include "debug.h"
#include "input/Forwarder.h"
#include "simparm/NodeHandle.h"
#include "input/Source.h"

namespace dStorm {
namespace input {

template <typename Type>
Forwarder<Type>::Forwarder() {}

template <typename Type>
Forwarder<Type>::Forwarder(const Forwarder& o) : Link<Type>(o)
{
    if ( o.more_specialized.get() )
        more_specialized.reset( o.more_specialized->clone() );
    if ( more_specialized.get() )
        connection = more_specialized->notify( 
            boost::bind( &Forwarder::traits_changed, this, _1, more_specialized.get() ) );
}

template <typename Type>
Forwarder<Type>::~Forwarder() { 
    DEBUG("Destructing " << this);
}

template <typename Type>
void Forwarder<Type>::registerNamedEntries(simparm::NodeHandle n) {
    if ( more_specialized.get() ) 
        more_specialized->registerNamedEntries(n);
}

template <typename Type>
std::unique_ptr<Source<Type>> Forwarder<Type>::upstream_source() { 
    assert( more_specialized.get() );
    return more_specialized->make_source();
}

template <typename Type>
Source<Type>* Forwarder<Type>::makeSource() { 
    return upstream_source().release();
}

template <typename Type>
void Forwarder<Type>::insert_new_node( std::unique_ptr<Link<Type>> n ) {
    if ( more_specialized.get() )
        more_specialized->insert_new_node(std::move(n));
    else
        insert_here(std::move(n));
}

template <typename Type>
typename Link<Type>::TraitsRef Forwarder<Type>::upstream_traits() const {
    assert( more_specialized.get() );
    return more_specialized->current_meta_info();
}

template <typename Type>
void Forwarder<Type>::traits_changed( TraitsRef ref, Link<Type>* l ) {
    DEBUG("Forwarder " << this << " got meta info update from " << l << " for " << ref.get() );
    this->update_current_meta_info(ref);
}

template <typename Type>
std::string Forwarder<Type>::name() const { 
    assert( more_specialized.get() );
    return more_specialized->name();
}

template <typename Type>
void Forwarder<Type>::insert_here( std::unique_ptr<Link<Type>> link ) {
    if ( more_specialized.get() )
        link->insert_new_node( std::move(more_specialized) );
    more_specialized = std::move(link);
    if ( more_specialized.get() ) {
        connection = more_specialized->notify( 
            boost::bind( &Forwarder::traits_changed, this, _1, more_specialized.get() ) );
    }
}

template <typename Type>
void Forwarder<Type>::publish_meta_info() {
    if ( ! more_specialized.get() )
        throw std::logic_error(name() + " needs a subinput to publish meta info");
    DEBUG(name() << " calls for publishment of meta info");
    more_specialized->publish_meta_info();
    DEBUG(name() << " called for publishment of meta info");
    if ( ! this->current_meta_info().get() )
        throw std::logic_error(name() + " did not publish meta info on request");
}

}
}

#endif
