#ifndef SIMPARM_CONFIG_ENTRY_CHOICE
#define SIMPARM_CONFIG_ENTRY_CHOICE

#include "Entry.h"
#include "Attribute.h"
#include "Choice.h"
#include <vector>
#include <map>
#include <stdexcept>

namespace simparm {

class ChoiceEntryBase
: public BasicEntry {
  protected:
    NodeHandle create_hidden_node( simparm::NodeHandle );
    static const std::string no_selection;

    std::vector< Choice* > choices;
    typedef std::map<std::string,Choice*> Entries;
    Entries entries;
    bool auto_select;

    NodeHandle current_ui;

    Choice* getElement( const std::string& key ) 
        { return &get_entry(key); }

    Choice& get_entry( const std::string &name )
    {
        typename Entries::iterator i = entries.find( name );
        if ( i == entries.end() )
            throw std::runtime_error("There is no choice named " + name + " for " + getDesc());
        else
            return *i->second;
    }
    const Choice& get_entry( const std::string &name ) const
    {
        typename Entries::const_iterator i = entries.find( name );
        if ( i == entries.end() )
            throw std::runtime_error("There is no choice named " + name + " for " + getDesc());
        else
            return *i->second;
    }

    NodeHandle make_naked_node( simparm::NodeHandle );
    void addChoice(Choice& choice);
    void removeChoice(Choice &choice);

    class ChoiceValidityChecker : public Attribute<string>::ChangeWatchFunction {
        string name;
        const Entries& entries;
        virtual bool operator()(const string& from, const string& to);
      public:
        ChoiceValidityChecker(const string& name, const Entries& entries)
            : name(name), entries(entries) {}
    };
    ChoiceValidityChecker checker;
  public:
    Attribute< std::string > value;

    ChoiceEntryBase(string name, string desc);
    ChoiceEntryBase(string name);
    ChoiceEntryBase(const ChoiceEntryBase& o);
    ~ChoiceEntryBase();
    ChoiceEntryBase& operator=
        (const ChoiceEntryBase& o);

    virtual ChoiceEntryBase* clone() const 
        { return new ChoiceEntryBase(*this); }

    void removeAllChoices(); 
    
    bool isValid() const { return value() != no_selection; }

    bool hasChoice( const std::string& name ) const 
        { return entries.find(name) != entries.end(); }

    void choose(const std::string &name)
        { value = name; }
    std::istream& readValue(std::istream& i) {
        std::string new_value;
        std::getline( i, new_value );
        if ( new_value == no_selection || hasChoice( new_value ) )
            value = new_value;
        else
            throw std::runtime_error("Invalid choice " + new_value + " for " + getName());
        return i;
    }

    /** Decide whether the first list entry should be automatically selected.
     *  If this behaviour is set to true (the default), the first entry in this
     *  choice entry will be selected automatically when no item was selected
     *  yet or the currently selected item is removed from the choice. Otherwise,
     *  the selection will be set to invalid. */
    void set_auto_selection( bool do_auto_select ) { 
        auto_select = do_auto_select;
        if ( do_auto_select && !isValid() && ! entries.empty() )
            value = entries.begin()->second->getName();
    }

    NodeHandle attach_ui( simparm::NodeHandle node );
};

template <typename ChoiceType>
struct ChoiceEntry 
: public ChoiceEntryBase
{
    ChoiceEntry(string name, string desc) : ChoiceEntryBase(name,desc) {}
    ChoiceEntry(string name) : ChoiceEntryBase(name) {}

    const ChoiceType& operator ()() const 
        { return static_cast<const ChoiceType&>( get_entry(value()) ); }
    ChoiceType& operator ()() 
        { return static_cast<ChoiceType&>( get_entry(value()) ); }
    const ChoiceType& active_choice() const { return (*this)(); }
    ChoiceType& active_choice() { return (*this)(); }

    /** Obsolete method accessing a choice by name. 
     *  \todo Remove this method when RegionSegmenter is cleaned up. */
    const ChoiceType& operator[](const std::string& name) const
        { return static_cast<const ChoiceType&>( get_entry(name) ); }
    /** Obsolete method accessing a choice by name. 
     *  \todo Remove this method when RegionSegmenter is cleaned up. */
    ChoiceType& operator[](const std::string& name)
        { return static_cast<ChoiceType&>( get_entry(name) ); }

    void setValue(const ChoiceType &choice) 
        { value = choice.getName(); }

    void addChoice(ChoiceType& choice) { ChoiceEntryBase::addChoice(choice); }
    void removeChoice(ChoiceType& choice) { ChoiceEntryBase::removeChoice(choice); }
};

}

#endif
