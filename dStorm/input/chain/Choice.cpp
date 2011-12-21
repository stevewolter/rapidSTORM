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
: simparm::Listener( simparm::Event::ValueChanged ),
  choices(name, desc), auto_select(auto_select)
{
    receive_changes_from( choices.value );
}

Choice::Choice(const Choice& o)
: Link(o), simparm::Listener( simparm::Event::ValueChanged ),
  choices(o.choices, simparm::NodeChoiceEntry<LinkAdaptor>::DeepCopy),
  my_traits(o.my_traits),
  auto_select(o.auto_select)
{
    DEBUG("Copied " << &o << " to " << this);
    receive_changes_from( choices.value );
    for ( simparm::NodeChoiceEntry<LinkAdaptor>::iterator i = choices.beginChoices(); i != choices.endChoices(); ++i )
        Choice::set_upstream_element( i->link(), *this, Add );
    publish_traits();
}

Choice::~Choice() {
    stop_receiving_changes_from(choices.value);
}
    
void Choice::operator()(const simparm::Event&)
{
    ost::MutexLock lock( global_mutex() );
    publish_traits();
}

Choice::AtEnd Choice::traits_changed( TraitsRef t, Link* from ) {
    Link::traits_changed(t, from);
    bool provides_something = ( t.get() != NULL && ! t->provides_nothing() );
    if ( auto_select && choices.isValid() && &choices.value().link() == from && ! provides_something ) {
        DEBUG("Auto-deselecting value other than the current");
        /* Choice can deliver no traits, i.e. is invalid. Find valid one. */
        bool found = false;
        for ( simparm::NodeChoiceEntry<LinkAdaptor>::iterator i = choices.beginChoices(); i != choices.endChoices(); ++i ) {
            if ( i->link().current_traits().get() != NULL && ! i->link().current_traits()->provides_nothing() ) {
                choices.value = &(*i);
                found = true;
            }
        }
        if ( ! found ) 
            choices.value = NULL;
    }
    if ( auto_select && ! choices.isValid() && provides_something ) {
        DEBUG("Auto-selecting " << from->link->getName() );
        for ( simparm::NodeChoiceEntry<LinkAdaptor>::iterator i = choices.beginChoices(); i != choices.endChoices(); ++i )
            if ( &i->link() == from )
                choices.value = &*i;
    }
    return publish_traits();
}

Choice::AtEnd Choice::publish_traits() {
    TraitsRef exemplar;
    if ( choices.isValid() && choices.value().link().current_traits().get() )
        exemplar = choices.value().link().current_traits();
    if ( exemplar.get() )
        my_traits.reset( new MetaInfo(*exemplar) );
    else
        my_traits.reset( new MetaInfo() );
    for ( simparm::NodeChoiceEntry<LinkAdaptor>::iterator i = choices.beginChoices(); i != choices.endChoices(); ++i ) {
        if ( i->link().current_traits() != exemplar && i->link().current_traits().get() )
            my_traits->forward_connections( *i->link().current_traits() );
    }
    return notify_of_trait_change( my_traits );
}

BaseSource* Choice::makeSource() {
    if ( ! choices.isValid() )
        throw std::runtime_error("No choice selected for '" + description() + "'");
    return choices.value().link().makeSource();
}

Choice* Choice::clone() const 
    { return new Choice(*this); }
void Choice::registerNamedEntries( simparm::Node& node ) {
    node.push_back( *this );
}

void Choice::add_choice( std::auto_ptr<Link> fresh ) 
{
    Link& l = *fresh;
    Choice::set_upstream_element( *fresh, *this, Add );
    std::auto_ptr< LinkAdaptor > adaptor( new LinkAdaptor(fresh) );
    adaptor->registerNamedEntries();
    choices.addChoice( choices.endChoices(), adaptor );
    traits_changed( l.current_traits(), &l );
}

Choice::LinkAdaptor::LinkAdaptor( std::auto_ptr<input::chain::Link> l ) 
    : node(l->name(), l->description()), _link(l) 
{
}

Choice::LinkAdaptor::~LinkAdaptor() {
    _link.reset();
}

void Choice::insert_new_node( std::auto_ptr<Link> l, Place ) {
    add_choice(l);
}

Link& Choice::get_first_link() { 
    return choices.beginChoices()->link(); 
}

}
}
}
