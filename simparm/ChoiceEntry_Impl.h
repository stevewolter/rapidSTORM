#ifndef SIMPARM_CHOICEENTRY_IMPL_HH
#define SIMPARM_CHOICEENTRY_IMPL_HH

#include "ChoiceEntry.h"
#include "debug.h"

namespace simparm {

template <typename ChoiceType>
const std::string ChoiceEntry<ChoiceType>::no_selection = "";

template <typename ChoiceType>
ChoiceEntry<ChoiceType>::
ChoiceEntry(string name, string desc)
: BasicEntry(name,desc),
  auto_select(true),
  value( "value", no_selection )
{
    SIMPARM_DEBUG(this << " constructed from scratch");
}


template <typename ChoiceType>
ChoiceEntry<ChoiceType>::
ChoiceEntry(const ChoiceEntry& o)
: BasicEntry(o), auto_select(o.auto_select), value(o.value)
{
    SIMPARM_DEBUG(this << "is constructed by copy");
}

template <typename ChoiceType>
ChoiceEntry<ChoiceType>::~ChoiceEntry() {
}

template <typename ChoiceType>
void ChoiceEntry<ChoiceType>::
addChoice(ChoiceType& choice)
{ 
    if ( current_ui )
        choice.attach_ui( current_ui );
    choices.push_back( &choice );
    entries.insert( std::make_pair( choice.getName(), &choice ) );
    if (auto_select && value() == no_selection ) {
        SIMPARM_DEBUG("Auto-selecting value");
        value = choice.getName();
    }
}

template <typename ChoiceType>
void ChoiceEntry<ChoiceType>::
removeChoice(ChoiceType &choice)
{
    bool auto_select_new_value = false;
    if ( value() == choice.getName() ) {
        value = no_selection;
        auto_select_new_value = this->auto_select;
    }

    choices.erase( std::remove( choices.begin(), choices.end(), &choice ) );
    entries.erase( choice.getName() );
    if ( auto_select_new_value && !choices.empty() )
        value = this->choices.front()->getName();
    if ( current_ui )
        choice.detach_ui( current_ui );
}

template <typename ChoiceType>
void ChoiceEntry<ChoiceType>::removeAllChoices()
{ 
    SIMPARM_DEBUG("Removing all choices");
    value = no_selection;
    if ( current_ui )
        for ( typename std::vector< ChoiceType* >::iterator i = choices.begin(); i != choices.end(); i++)
        {
            (*i)->detach_ui( current_ui );
        }
    entries.clear();
    choices.clear();
}

template <typename ChoiceType>
NodeHandle ChoiceEntry<ChoiceType>::create_hidden_node( simparm::NodeHandle n ) {
    NodeHandle r = BasicEntry::create_hidden_node( n );
    value.add_to ( r );
    current_ui = r;
    for ( typename std::vector< ChoiceType* >::iterator i = choices.begin(); i != choices.end(); ++i ) {
        (*i)->attach_ui( current_ui );
    }
    return r;
}

template <typename ChoiceType>
NodeHandle 
ChoiceEntry<ChoiceType>::make_naked_node( simparm::NodeHandle n ) {
    return create_choice( n, getName() );
}

}

#endif
