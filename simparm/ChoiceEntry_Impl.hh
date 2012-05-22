#ifndef SIMPARM_CHOICEENTRY_IMPL_HH
#define SIMPARM_CHOICEENTRY_IMPL_HH

#include "ChoiceEntry.hh"
#include "debug.hh"

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
        choice.attach_ui( *current_ui );
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

    entries.erase( choice.getName() );
    if ( auto_select_new_value && !entries.empty() )
        value = this->entries.begin()->first;
    if ( current_ui )
        choice.detach_ui( *current_ui );
}

template <typename ChoiceType>
void ChoiceEntry<ChoiceType>::removeAllChoices()
{ 
    SIMPARM_DEBUG("Removing all choices");
    value = no_selection;
    if ( current_ui )
        for ( typename Entries::iterator i = entries.begin(); i != entries.end(); i++)
        {
            i->second->detach_ui( *current_ui );
        }
    entries.clear();
}

#if 0
template <typename ChoiceType>
std::list<std::string> 
ChoiceEntry<ChoiceType>::printValues() const
{
    SIMPARM_DEBUG("Printing values");
    std::list<std::string> children = Object::printValues();
    children.push_front( getName() + " = " + value() );
    return children;
}

template <typename ChoiceType>
void ChoiceEntry<ChoiceType>::
    printHelp(ostream &o) const 
{
    SIMPARM_DEBUG("Printing help");
    BasicEntry::printHelp(o);
    formatParagraph(o, 0, 22, "");
    string s = "Choices are: ";
    for (typename Entries::const_iterator i= entries.begin();
            i !=  entries.end(); i++)
    {
        if (value() == i->first) s += "_";
        s += i->first;
        if (value() == i->first) s += "_";
        s += " ";
    }
    formatParagraph(o, 23, 79, s);
    o << "\n";
}
#endif

template <typename ChoiceType>
NodeRef ChoiceEntry<ChoiceType>::create_hidden_node( simparm::Node& n ) {
    NodeRef r = BasicEntry::create_hidden_node( n );
    r.add_attribute( value );
    current_ui = r;
    for ( typename Entries::iterator i = entries.begin(); i != entries.end(); ++i )
        i->second->attach_ui( *current_ui );
    return r;
}

}

#endif
