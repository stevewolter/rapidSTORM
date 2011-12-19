#include "../debug.h"

#include "Alternatives.h"
#include "Forwarder.h"
#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/Message.hh>
#include <map>
#include "Forwarder.h"
#include "../InputMutex.h"
#include <dStorm/input/chain/MetaInfo.h>

namespace dStorm {
namespace input {
namespace chain {

class Alternatives::UpstreamCollector 
: public Forwarder, boost::noncopyable
{
    TraitsRef traits;
    Alternatives& papa;

  public:
    UpstreamCollector(Alternatives& papa) : papa(papa) {}
    UpstreamCollector(Alternatives& papa, const UpstreamCollector& o)
        : traits(o.traits), papa(papa) {}

    UpstreamCollector* clone() const 
        { assert(false); throw std::logic_error("UpstreamCollector unclonable"); }

    BaseSource* makeSource() { return Forwarder::makeSource(); }

    AtEnd traits_changed( TraitsRef r, Link* from ) {
        /* TODO: This method needs to be aware of traits-run-throughs during updates. */
        DEBUG("Traits changed from upstream in " << this  << " from " << from << " to " << r.get() );
        Link::traits_changed(r,from);
        traits = r;
        AtEnd rv;
        for ( Alternatives::iterator i = papa.beginChoices(); i != papa.endChoices(); ++i )
            rv = i->traits_changed( r, this );
        return rv;
    }

    void set_upstream_element( Link& element, SetType type ) {
        if ( type == Add ) {
            assert( papa.hasChoice( element.getNode().getName() ) );
        }
    }
};

Alternatives::Alternatives(std::string name, std::string desc)
: simparm::NodeChoiceEntry<Forwarder>(name, desc),
  simparm::Listener( simparm::Event::ValueChanged ),
  collector( new UpstreamCollector(*this) )
{
    simparm::NodeChoiceEntry<Forwarder>::set_auto_selection( false );
    receive_changes_from( value );
}

Alternatives::Alternatives(const Alternatives& o)
: Link(o), simparm::NodeChoiceEntry<Forwarder>(o, simparm::NodeChoiceEntry<Forwarder>::NoCopy),
  simparm::Listener( simparm::Event::ValueChanged ),
  collector( new UpstreamCollector(*this, *o.collector) )
{
    receive_changes_from( value );
}

Alternatives::~Alternatives() {
    stop_receiving_changes_from(value);
}

void Alternatives::set_more_specialized_link_element( Link* l ) {
    collector->set_more_specialized_link_element( l );
}
    
    
void Alternatives::operator()(const simparm::Event&)
{
    ost::MutexLock lock( global_mutex() );
    DEBUG("Value changed for alternatives");
    if ( value.hasValue() ) {
        DEBUG("Propagating traits for new value");
        notify_of_trait_change( value().current_traits() );
    }
}

Link::AtEnd Alternatives::traits_changed( TraitsRef t, Link* from ) {
    Link::traits_changed(t,from);
    DEBUG("Alternatives " << this << " traits changed to " << t.get() << " from " << from->getNode().getName());
    bool has_a_value = t.get() != NULL && ! t->provides_nothing();
    if ( &( value() ) == from && ! has_a_value ) {
        DEBUG("Auto-deselecting value other than the current");
        /* Choice can deliver no traits, i.e. is invalid. Find valid one. */
        bool found = false;
        for ( iterator i = beginChoices(); i != endChoices(); ++i ) {
            if ( i->current_traits().get() != NULL && ! i->current_traits()->provides_nothing() ) {
                value = &(*i);
                found = true;
            }
        }
        if ( ! found ) 
            value = NULL;
    } else if ( ! isValid() && has_a_value )
    {
        value = dynamic_cast<Forwarder*>(from);
    }
    DEBUG("Finished possible auto-deselection");
    if ( from == &( value() ) ) {
        DEBUG("Notifying of trait change for traits " << t.get());
        return notify_of_trait_change( t );
        DEBUG("Notified of trait change");
    } else {
        /* Reporting choice is not current choice */
        return AtEnd();
    }
}

BaseSource* Alternatives::makeSource() {
    if ( ! isValid() ) throw std::runtime_error("No alternative selected for '" + getDesc() + "'");
    return value().makeSource();
}

Alternatives* Alternatives::clone() const 
    { return new Alternatives(*this); }
simparm::Node& Alternatives::getNode() {
    return *this;
}

void Alternatives::add_choice( Forwarder& choice, ChoiceEntry::iterator where) 
{
    this->addChoice( where, choice );
    Alternatives::set_upstream_element( choice, *this, Add );
    choice.set_more_specialized_link_element( collector.get() );
    traits_changed( choice.current_traits(), &choice );
}
void Alternatives::push_back_choice( Forwarder& c) {
    add_choice( c, endChoices() );
}

void Alternatives::remove_choice( Forwarder& choice ) {
    choice.set_more_specialized_link_element( NULL );
    Alternatives::set_upstream_element( choice, *this, Remove );
    this->removeChoice( choice );
}

void Alternatives::downstream_element_destroyed( Link& which )
{
    /* Only a static cast here since the Forwarder part of the object *
     * is most probably already destroyed. */
    removeChoice_noNode( static_cast<Forwarder&>(which) );
}

void Alternatives::throw_exception_for_invalid_configuration() const
{
    throw std::runtime_error("No alternative chosen for '" + getDesc() + "'");
}

void Alternatives::insert_new_node( std::auto_ptr<Link> link, Place p )
{
    collector->insert_new_node(link,p);
}

}
}
}
