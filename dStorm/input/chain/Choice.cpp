#include "Choice.h"
#include "Context.h"
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
#if 0
    /* Code here does not get executed because no entries were copied */
    for ( iterator i = beginChoices(); i != endChoices(); ++i ) {
        set_upstream_element( *i, *this, Add );
        traits_changed( i->current_traits(), &*i );
    }
#endif
    receive_changes_from( value );
    for ( boost::ptr_vector< Link >::const_iterator l = o.choices.begin(); l != o.choices.end(); ++l ) {
        choices.push_back( l->clone() );
        addChoice(endChoices(), choices.back());
        Choice::set_upstream_element( choices.back(), *this, Add );
    }
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
    if ( auto_select && ! isValid() && ( t.get() != NULL && ! t->provides_nothing() ) )
        value = *from;
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

void Choice::add_choice( Link& choice, ChoiceEntry::iterator where) 
{
    this->addChoice( where, choice );
    Choice::set_upstream_element( choice, *this, Add );
    traits_changed( choice.current_traits(), &choice );
}
void Choice::insert_new_node( std::auto_ptr<Link> c, Place ) {
    add_choice( *c, endChoices() );
    TraitsRef nt = c->current_traits();
    Link* nc = c.get();
    choices.push_back( c );
    traits_changed( nt, nc );
}

void Choice::remove_choice( Link& choice ) {
    Choice::set_upstream_element( choice, *this, Remove );
    this->removeChoice( choice );
}

}
}
}
