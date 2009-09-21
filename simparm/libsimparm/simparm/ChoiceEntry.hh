#ifndef SIMPARM_CONFIG_ENTRY_CHOICE
#define SIMPARM_CONFIG_ENTRY_CHOICE

#include "Entry.hh"
#include <vector>
#include <map>
#include <set>
#include <stdexcept>
#include <algorithm>
#include <cassert>

namespace simparm {

extern void formatParagraph(ostream &o, unsigned int left_col, 
                unsigned int right_col, const string &s);

template <typename ChoiceType>
class NodeChoiceEntry 
: public Entry,
  protected Attribute<ChoiceType*>::NameMap
{
  public:
    typedef std::map<std::string, ChoiceType*> EntriesByName;
    typedef std::set<ChoiceType*> Entries;
  protected:
    Entries entries;
    EntriesByName entriesByName;
    bool auto_select;

    struct ValidityChecker 
        : public Attribute<ChoiceType*>::ChangeWatchFunction 
    {
        const Entries& entries;
        ValidityChecker(const Entries& entries) : entries(entries) {}
        virtual bool operator()(const ChoiceType*,const ChoiceType* to) { 
            if ( to == NULL )
                return true;
            else if ( entries.find( const_cast<ChoiceType*>(to) ) 
                 == entries.end() ) 
            {
                throw std::logic_error("Invalid choice.");
            } else
                return true;
        }
    };
    ValidityChecker choiceChecker;

    ChoiceType* getElement( const std::string& key ) 

    {
        return get_entry(key);
    }

    std::string getNameForElement( ChoiceType *element ) {
        return element->getName();
    }

    ChoiceType* get_entry( const std::string &name )

    {
        typename EntriesByName::iterator i = entriesByName.find( name );
        if ( i == entriesByName.end() )
            throw std::invalid_argument("There is no choice named " + name);
        else
            return i->second;
    }
    const ChoiceType* get_entry( const std::string &name ) const

    {
        typename EntriesByName::const_iterator i = entriesByName.find( name );
        if ( i == entriesByName.end() )
            throw std::invalid_argument("There is no choice named " + name);
        else
            return i->second;
    }

  public:
    Attribute< ChoiceType* > value;

    inline NodeChoiceEntry(string name, string desc);

    enum EntryDuplicationPolicy { 
        NoCopy, ShallowCopy, DeepCopy };

    inline NodeChoiceEntry(
        const NodeChoiceEntry<ChoiceType>& o,
        EntryDuplicationPolicy duplicationPolicy = DeepCopy);
    inline ~NodeChoiceEntry();
    inline NodeChoiceEntry& operator=
        (const NodeChoiceEntry<ChoiceType>& o);

    virtual NodeChoiceEntry<ChoiceType>* clone() const 
        { return new NodeChoiceEntry(*this); }
    std::string getTypeDescriptor() const  
        { return "ChoiceEntry"; }

    inline void addChoice(ChoiceType& choice);
    inline void addChoice(std::auto_ptr<ChoiceType> choice);
    /** Syntactic sugar: Equivalent to the auto_ptr-call. */
    void addChoice(ChoiceType* choice)
        { this->addChoice(std::auto_ptr<ChoiceType>(choice)); }

    inline void removeChoice(ChoiceType &choice) 
;
    inline void removeAllChoices(); 
    
    bool isValid() const { return value.hasValue(); }

    const ChoiceType& operator ()() const 
        { return value();}

    const ChoiceType& operator[](const std::string& name) const

        { typename EntriesByName::iterator i = entriesByName.find( name );
          if ( i != entriesByName.end()) return *i->second;
          else throw std::invalid_argument("No choice named " + name); }
    ChoiceType& operator[](const std::string& name)

        { typename EntriesByName::iterator i = entriesByName.find( name );
          if ( i != entriesByName.end()) return *i->second;
          else throw std::invalid_argument("No choice named " + name); }
    bool hasChoice( const std::string& name ) const 
        { return entriesByName.find(name) != entriesByName.end(); }

    inline void setValue(const ChoiceType &choice) 
        { value = const_cast<ChoiceType*>(&choice); }
    inline void setValue(const ChoiceType *choice) 
        { value = const_cast<ChoiceType*>(choice); }
    void choose(const std::string &name)
        { value = (*this)[name]; }
    istream& readValue(istream& i) {
        i >> value;
        return i;
    }

    std::list<std::string> printValues() const;
    void printHelp(ostream &o) const;

    /** Decide whether the first list entry should be automatically selected.
     *  If this behaviour is set to true (the default), the first entry in this
     *  choice entry will be selected automatically when no item was selected
     *  yet or the currently selected item is removed from the choice. Otherwise,
     *  the selection will be set to invalid. */
    void set_auto_selection( bool do_auto_select ) { 
        auto_select = do_auto_select;
        if ( do_auto_select && !isValid() && ! entries.empty() )
            value = *entries.begin();
    }

    class const_iterator;
    class iterator
        : public std::iterator< 
            typename Entries::const_iterator::iterator_category,
            const ChoiceType>
    {
        typename Entries::iterator base;
        friend class const_iterator;
      public:
        iterator(typename Entries::iterator base)
            : base(base) {}

        ChoiceType& operator*() const 
            { return **base; }
        ChoiceType* operator->() const 
            { return *base; }
        iterator& operator++() { ++base; return *this; }
        iterator& operator++(int) { base++; return *this; }
        iterator& operator--() { --base; return *this; }
        iterator& operator--(int) { base--; return *this; }

        bool operator==(const iterator& o)
            { return o.base == base; }
        bool operator!=(const iterator& o)
            { return o.base != base; }
    };
    class const_iterator
        : public std::iterator< 
            typename Entries::const_iterator::iterator_category,
            const ChoiceType>
    {
        typename Entries::const_iterator base;
      public:
        const_iterator(typename Entries::const_iterator base)
            : base(base) {}

        const_iterator(const iterator& ci)
            : base(ci.base) {}
        const_iterator& operator=(const iterator& ci)
            { base = (ci.base); return *this; }

        const ChoiceType& operator*() const 
            { return **base; }
        const ChoiceType* operator->() const 
            { return *base; }
        const_iterator& operator++() { ++base; return *this; }
        const_iterator& operator++(int) { base++; return *this; }
        const_iterator& operator--() { --base; return *this; }
        const_iterator& operator--(int) { base--; return *this; }

        bool operator==(const const_iterator& o)
            { return o.base == base; }
        bool operator!=(const const_iterator& o)
            { return o.base != base; }
    };
    const_iterator beginChoices() const 
        {return const_iterator(entries.begin());}
    const_iterator endChoices() const 
        {return const_iterator(entries.end());}
    iterator beginChoices() 
        {return iterator(entries.begin());}
    iterator endChoices() 
        {return iterator(entries.end());}
    int numChoices() const { return entries.size(); }
};

template <typename DataType>
class DataChoice : public Object {
  private:
    DataType num;
  public:
    DataChoice(DataType num, string name, string desc)
        : Object(name, desc), num(num) {}
    DataChoice<DataType>* clone() const
        { return new DataChoice<DataType>(*this); }

    const DataType& operator()() const { return num; }
    DataType& operator()() { return num; }
};

template <typename DataType>
class DataChoiceEntry 
: public NodeChoiceEntry< DataChoice<DataType> >  
{
  public:
    inline DataChoiceEntry(string name, string desc);
    inline virtual DataChoiceEntry<DataType>* clone() const;
    inline ~DataChoiceEntry();

    inline void addChoice(DataType num, string name, string desc) 
;

    DataChoiceEntry<DataType>&
        operator=(const DataType& choice) 
    {
        for (typename NodeChoiceEntry< DataChoice<DataType> >::
                iterator i = this->beginChoices();
                                     i != this->endChoices(); i++)
            if ( (*i)() == choice ) {
                this->value = &*i;
                break;
            }
        return *this;
    }

    const DataType& operator()() const 
        { return this->value()(); }
    DataType& operator()() {
        DataChoice<DataType>& v = this->value();
        return v();
    }
};

class ChoiceEntry : public DataChoiceEntry<int> {
  public:
    inline ChoiceEntry(string name, string desc);
    inline ~ChoiceEntry();
    inline virtual ChoiceEntry* clone() const;

    inline void addChoice(int pos, string name, string desc);
    inline void addChoice(string name, string desc);

    ChoiceEntry& operator=(const string& name) 
        { choose(name); return *this; }
    ChoiceEntry& operator=(int id) 
        { (*(DataChoiceEntry<int>*)this) = id; return *this; }
};

}

#endif
