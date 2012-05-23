#include "debug.h"
#include "Choice.h"
#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/Message.hh>
#include <dStorm/input/InputMutex.h>
#include <dStorm/input/MetaInfo.h>
#include <boost/foreach.hpp>

namespace boost {

using dStorm::input::Choice;

template <>
inline Choice::LinkAdaptor* new_clone<Choice::LinkAdaptor>( const Choice::LinkAdaptor& l )
    { return l.clone(); }
template <>
inline void delete_clone<Choice::LinkAdaptor>(const Choice::LinkAdaptor* l) 
    { delete l; }

}

namespace dStorm {
namespace input {

Choice::Choice(std::string name, std::string desc, bool auto_select)
: choices(name, desc), auto_select(auto_select)
{
}

Choice::Choice(const Choice& o)
: Link(o), 
  choices(o.choices),
  my_traits(o.my_traits),
  auto_select(o.auto_select)
{
    DEBUG("Copied " << &o << " to " << this);
    for ( iterator i = choices.begin(); i != choices.end(); ++i ) {
        i->connect(*this);
    }
}

Choice::~Choice() {
}
    
void Choice::publish_traits_locked()
{
    InputMutexGuard lock( global_mutex() );
    publish_traits();
}

void Choice::traits_changed( TraitsRef t, Link* from ) {
    bool provides_something = ( t.get() != NULL && ! t->provides_nothing() );
    if ( auto_select && choices.isValid() && &choices().link() == from && ! provides_something ) {
        DEBUG("Auto-deselecting value other than the current");
        /* Choice can deliver no traits, i.e. is invalid. Find valid one. */
        bool found = false;
        for ( iterator i = choices.begin(); i != choices.end(); ++i ) {
            if ( i->link().current_meta_info().get() != NULL && ! i->link().current_meta_info()->provides_nothing() ) {
                choices.value = i->getName();
                found = true;
            }
        }
        if ( ! found ) 
            choices.value = "";
    }
    if ( auto_select && ! choices.isValid() && provides_something ) {
        DEBUG("Auto-selecting " << from->link->getName() );
        for ( iterator i = choices.begin(); i != choices.end(); ++i )
            if ( &i->link() == from )
                choices.value = i->getName();
    }
    publish_traits();
}

void Choice::publish_traits() {
    TraitsRef exemplar;
    if ( choices.isValid() && choices().link().current_meta_info().get() )
        exemplar = choices().link().current_meta_info();
    if ( exemplar.get() )
        my_traits.reset( new MetaInfo(*exemplar) );
    else
        my_traits.reset( new MetaInfo() );
    for ( iterator i = choices.begin(); i != choices.end(); ++i ) {
        if ( i->link().current_meta_info() != exemplar && i->link().current_meta_info().get() )
            my_traits->forward_connections( *i->link().current_meta_info() );
    }
    update_current_meta_info( my_traits );
}

BaseSource* Choice::makeSource() {
    if ( ! choices.isValid() )
        throw std::runtime_error("No choice selected for '" + description() + "'");
    return choices().link().make_source().release();
}

Choice* Choice::clone() const 
    { return new Choice(*this); }
void Choice::registerNamedEntries( simparm::NodeHandle node ) {
    value_change_listen = choices.value.notify_on_value_change( boost::bind( &Choice::publish_traits_locked, this ) );
    choices.attach_ui( node );
}

void Choice::add_choice( std::auto_ptr<Link> fresh ) 
{
    std::auto_ptr< LinkAdaptor > adaptor( new LinkAdaptor(fresh) );
    adaptor->connect( *this );
    choices.addChoice( adaptor );
}

Choice::LinkAdaptor::LinkAdaptor( std::auto_ptr<input::Link> l ) 
    :  _link(l) 
{
}

Choice::LinkAdaptor::~LinkAdaptor() {
    _link.reset();
}

void Choice::insert_new_node( std::auto_ptr<Link> l, Place p ) {
    choices.begin()->link().insert_new_node(l,p); 
}

void Choice::publish_meta_info() {
    for ( iterator i = choices.begin(); i != choices.end(); ++i ) {
        i->link().publish_meta_info();
    }
    publish_traits();
    if ( ! current_meta_info().get() )
        throw std::logic_error(name() + " did not publish meta info on request");
}

}
}
