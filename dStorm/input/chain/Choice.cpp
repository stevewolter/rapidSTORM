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
  current_context(o.current_context),
  no_throw_context(o.no_throw_context),
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
    if ( value.hasValue() )
        publish_traits( value().current_traits() );
}

Choice::AtEnd Choice::traits_changed( TraitsRef t, Link* from ) {
    Link::traits_changed(t, from);
    if ( auto_select && &( value() ) == NULL && ( t.get() != NULL && ! t->provides_nothing() ) )
        value = *from;
    if ( from == &( value() ) )
        return publish_traits( t );
    else {
        /* The traits are not needed since its source is not
         * the current choice. */
        return AtEnd();
    }
}

Choice::AtEnd Choice::publish_traits( TraitsRef t ) {
    if ( t.get() != NULL ) {
        my_traits.reset( new MetaInfo(*t) );
        BOOST_FOREACH( const Link& link, choices )
            my_traits->forward_connections( link.current_traits() );
    } else {
        my_traits.reset();
    }
    return notify_of_trait_change( my_traits );
}

Choice::AtEnd Choice::context_changed( ContextRef context, Link* link ) {
    Link::context_changed(context, link);
    current_context = context;
    if ( context.get() ) {
        no_throw_context.reset( context->clone() );
        no_throw_context->throw_errors = false;
    } else {
        no_throw_context.reset();
    }
    AtEnd rv;
    for ( iterator i = beginChoices(); i != endChoices(); ++i ) {
        if ( &value() == &*i )
            rv = i->context_changed( current_context, this );
        else
            rv = i->context_changed( no_throw_context, this );
    }
    if ( context->throw_errors && !isValid() )
        throw std::runtime_error("No choice selected for '" + getDesc() + "'");
    return rv;
}

BaseSource* Choice::makeSource() {
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
    if ( no_throw_context.get() != NULL )
        choice.context_changed( no_throw_context, this );
    traits_changed( choice.current_traits(), &choice );
}
void Choice::insert_new_node( std::auto_ptr<Link> c) {
    add_choice( *c, endChoices() );
    choices.push_back( c );
}

void Choice::remove_choice( Link& choice ) {
    Choice::set_upstream_element( choice, *this, Remove );
    this->removeChoice( choice );
}

}
}
}
