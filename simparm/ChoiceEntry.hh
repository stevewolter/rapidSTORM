#ifndef SIMPARM_CONFIG_ENTRY_CHOICE
#define SIMPARM_CONFIG_ENTRY_CHOICE

#include "Entry.hh"
#include <vector>
#include <map>
#include <set>
#include <stdexcept>
#include <algorithm>
#include <cassert>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/tag.hpp>
#include "NodeHandle.hh"

namespace simparm {

extern void formatParagraph(ostream &o, unsigned int left_col, 
                unsigned int right_col, const string &s);

template <typename ChoiceType>
class ChoiceEntry 
: public BasicEntry
{
  protected:
    NodeRef create_hidden_node( simparm::Node& );
    static const std::string no_selection;

    typedef std::map<std::string,ChoiceType*> Entries;
    Entries entries;
    bool auto_select;

    NodeHandle current_ui;

    ChoiceType* getElement( const std::string& key ) 
        { return &get_entry(key); }

    ChoiceType& get_entry( const std::string &name )
    {
        typename Entries::iterator i = entries.find( name );
        if ( i == entries.end() )
            throw std::runtime_error("There is no choice named " + name + " for " + getDesc());
        else
            return *i->second;
    }
    const ChoiceType& get_entry( const std::string &name ) const
    {
        typename Entries::const_iterator i = entries.find( name );
        if ( i == entries.end() )
            throw std::runtime_error("There is no choice named " + name + " for " + getDesc());
        else
            return *i->second;
    }

    std::auto_ptr<Node> make_naked_node( simparm::Node& );

  public:
    Attribute< std::string > value;

    inline ChoiceEntry(string name, string desc);
    inline ChoiceEntry(const ChoiceEntry& o);
    inline ~ChoiceEntry();
    inline ChoiceEntry& operator=
        (const ChoiceEntry<ChoiceType>& o);

    virtual ChoiceEntry<ChoiceType>* clone() const 
        { return new ChoiceEntry(*this); }

    inline void addChoice(ChoiceType& choice) ;
    inline void removeChoice(ChoiceType &choice);
    inline void removeAllChoices(); 
    
    bool isValid() const { return value() != no_selection; }

    const ChoiceType& operator ()() const { return get_entry(value()); }
    ChoiceType& operator ()() { return get_entry(value()); }
    const ChoiceType& active_choice() const { return get_entry(value()); }
    ChoiceType& active_choice() { return get_entry(value()); }

    const ChoiceType& operator[](const std::string& name) const
        { return get_entry(name); }
    ChoiceType& operator[](const std::string& name)
        { return get_entry(name); }
    bool hasChoice( const std::string& name ) const 
        { return entries.find(name) != entries.end(); }

    inline void setValue(const ChoiceType &choice) 
        { value = choice.getName(); }
    void choose(const std::string &name)
        { value = name; }
    istream& readValue(istream& i) {
        std::string new_value;
        std::getline( i, new_value );
        if ( new_value == no_selection || hasChoice( new_value ) )
            value = new_value;
        else
            throw std::runtime_error("Invalid choice " + new_value + " for " + getName());
        return i;
    }

    void printHelp(ostream &o) const;

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
};

}

#endif
