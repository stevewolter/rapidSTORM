#include "debug.h"
#include "Choice.h"
#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/Message.hh>
#include <dStorm/input/InputMutex.h>
#include <dStorm/input/chain/MetaInfo.h>
#include <boost/foreach.hpp>

namespace dStorm {
namespace input {
namespace chain {

Choice::Choice(std::string name, std::string desc, bool auto_select)
: simparm::NodeChoiceEntry<Link>(name, desc),
  simparm::Listener( simparm::Event::ValueChanged ),
  auto_select(auto_select)
{
    receive_changes_from( value );
}

Choice::Choice(const Choice& o)
: Link(o), simparm::NodeChoiceEntry<Link>(o, simparm::NodeChoiceEntry<Link>::NoCopy),
  simparm::Listener( simparm::Event::ValueChanged ),
  auto_select(o.auto_select),
  choices()
{
    DEBUG("Copied " << &o << " to " << this);
#if 0
    /* Code here does not get executed because no entries were copied */
    for ( iterator i = beginChoices(); i != endChoices(); ++i ) {
        set_upstream_element( *i, *this, Add );
        traits_changed( i->current_traits(), &*i );
    }
#endif
    receive_changes_from( value );
    for ( boost::ptr_vector< Link >::const_iterator l = o.choices.begin(); l != o.choices.end(); ++l )
        add_choice( std::auto_ptr<Link>(l->clone()) );

    if ( o.isValid() )
        choose( o.value().getNode().getName() );
}

Choice::~Choice() {
    stop_receiving_changes_from(value);
    for ( iterator i = beginChoices(); i != endChoices(); ++i ) {
        set_upstream_element( *i, *this, Remove );
    }
    this->simparm::NodeChoiceEntry<Link>::removeAllChoices();
}
    
void Choice::operator()(const simparm::Event&)
{
    ost::MutexLock lock( global_mutex() );
    publish_traits();
}

Choice::AtEnd Choice::traits_changed( TraitsRef t, Link* from ) {
    Link::traits_changed(t, from);
    bool provides_something = ( t.get() != NULL && ! t->provides_nothing() );
    if ( auto_select && &( value() ) == from && ! provides_something ) {
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
    }
    if ( auto_select && ! isValid() && provides_something ) {
        DEBUG("Auto-selecting " << from->getNode().getName() );
        value = from;
    }
    return publish_traits();
}

Choice::AtEnd Choice::publish_traits() {
    TraitsRef exemplar;
    if ( this->isValid() && value().current_traits().get() )
        exemplar = value().current_traits();
    if ( exemplar.get() )
        my_traits.reset( new MetaInfo(*exemplar) );
    else
        my_traits.reset( new MetaInfo() );
    BOOST_FOREACH( const Link& link, choices ) {
        if ( link.current_traits() != exemplar && link.current_traits().get() )
            my_traits->forward_connections( *link.current_traits() );
    }
    return notify_of_trait_change( my_traits );
}

BaseSource* Choice::makeSource() {
    if ( ! isValid() )
        throw std::runtime_error("No choice selected for '" + getDesc() + "'");
    return value().makeSource();
}

Choice* Choice::clone() const 
    { return new Choice(*this); }
simparm::Node& Choice::getNode() {
    return *this;
}

void Choice::add_choice( std::auto_ptr<Link> fresh ) 
{
    Link& l = *fresh;
    ChoiceEntry::addChoice( ChoiceEntry::endChoices(), *fresh );
    Choice::set_upstream_element( *fresh, *this, Add );
    choices.push_back( fresh );
    traits_changed( l.current_traits(), &l );
}

void Choice::insert_new_node( std::auto_ptr<Link> l, Place ) {
    add_choice(l);
}

}
}
}
