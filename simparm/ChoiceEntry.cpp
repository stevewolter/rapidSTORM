#include "ManagedChoiceEntry.h"
#include "Object.h"
#include <simparm/dummy_ui/fwd.h>

namespace simparm {

const std::string ChoiceEntryBase::no_selection = "";

ChoiceEntryBase::ChoiceEntryBase(string name, string desc)
: BasicEntry(name,desc),
  auto_select(true),
  value( "value", no_selection )
{
}


ChoiceEntryBase::ChoiceEntryBase(const ChoiceEntryBase& o)
: BasicEntry(o), auto_select(o.auto_select), value(o.value)
{
}

ChoiceEntryBase::~ChoiceEntryBase() {}

void ChoiceEntryBase::addChoice(Choice& choice)
{ 
    if ( current_ui )
        choice.attach_ui( current_ui );
    choices.push_back( &choice );
    entries.insert( std::make_pair( choice.getName(), &choice ) );
    if (auto_select && value() == no_selection ) {
        value = choice.getName();
    }
}

void ChoiceEntryBase::removeChoice(Choice &choice)
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
        choice.attach_ui( simparm::dummy_ui::make_node() );
}

void ChoiceEntryBase::removeAllChoices()
{ 
    value = no_selection;
    if ( current_ui )
        for ( typename std::vector< Choice* >::iterator i = choices.begin(); i != choices.end(); i++)
        {
            (*i)->attach_ui( simparm::dummy_ui::make_node() );
        }
    entries.clear();
    choices.clear();
}

NodeHandle ChoiceEntryBase::create_hidden_node( simparm::NodeHandle n ) {
    NodeHandle r = BasicEntry::create_hidden_node( n );
    value.add_to ( r );
    return r;
}

NodeHandle 
ChoiceEntryBase::make_naked_node( simparm::NodeHandle n ) {
    return create_choice( n, getName() );
}

NodeHandle ChoiceEntryBase::attach_ui( NodeHandle node ) {
    NodeHandle h = Object::attach_ui( node );
    current_ui = h;
    for ( typename std::vector< Choice* >::iterator i = choices.begin(); i != choices.end(); ++i ) {
        (*i)->attach_ui( current_ui );
    }
    return h;
}


struct DummyChoice : public Choice {
    DummyChoice* clone() const { return new DummyChoice(); }
    std::string getName() const { return "Dummy"; }
    void attach_ui( simparm::NodeHandle ) {}
};

template class ChoiceEntry<DummyChoice>;
template class ManagedChoiceEntry<DummyChoice>;

}
