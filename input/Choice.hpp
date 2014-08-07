#include "debug.h"
#include "input/Choice.h"
#include "simparm/ChoiceEntry.h"
#include "simparm/Message.h"
#include "input/InputMutex.h"
#include "input/MetaInfo.h"
#include "input/Source.h"

namespace dStorm {
namespace input {

template <typename Type>
Choice<Type>::Choice(std::string name, bool auto_select)
: choices(name), auto_select(auto_select), will_publish_traits(false)
{
}

template <typename Type>
Choice<Type>::Choice(const Choice& o)
: Link<Type>(o), 
  choices(o.choices),
  my_traits(o.my_traits),
  auto_select(o.auto_select), 
  will_publish_traits(false)
{
    DEBUG("Copied " << &o << " to " << this);
    for ( iterator i = choices.begin(); i != choices.end(); ++i ) {
        i->connect(*this);
    }
}

template <typename Type>
void Choice<Type>::publish_traits_locked()
{
    InputMutexGuard lock( global_mutex() );
    publish_traits();
}

template <typename Type>
void Choice<Type>::traits_changed( TraitsRef t, Link<Type>* from ) {
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
        for ( iterator i = choices.begin(); i != choices.end(); ++i )
            if ( &i->link() == from )
                choices.value = i->getName();
    }
    if ( ! will_publish_traits )
        publish_traits();
}

template <typename Type>
void Choice<Type>::publish_traits() {
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

template <typename Type>
BaseSource* Choice<Type>::makeSource() {
    if ( ! choices.isValid() )
        throw std::runtime_error("No choice selected for '" + description() + "'");
    return choices().link().make_source().release();
}

template <typename Type>
Choice* Choice<Type>::clone() const {
    return new Choice(*this);
}

template <typename Type>
void Choice<Type>::registerNamedEntries( simparm::NodeHandle node ) {
    value_change_listen = choices.value.notify_on_value_change( boost::bind( &Choice<Type>::publish_traits_locked, this ) );
    choices.attach_ui( node );
}

template <typename Type>
void Choice<Type>::add_choice( std::unique_ptr<Link<Type>> fresh ) {
    std::unique_ptr< LinkAdaptor > adaptor( new LinkAdaptor(std::move(fresh)) );
    adaptor->connect( *this );
    choices.addChoice( std::move(adaptor) );
}

template <typename Type>
void Choice<Type>::insert_new_node( std::unique_ptr<Link<Type>> l ) {
    choices.begin()->link().insert_new_node(std::move(l)); 
}

template <typename Type>
void Choice::publish_meta_info() {
    will_publish_traits = true;
    for ( iterator i = choices.begin(); i != choices.end(); ++i ) {
        i->link().publish_meta_info();
    }
    will_publish_traits = false;
    publish_traits();
    assert( current_meta_info().get() );
}

}
}
