#ifndef SIMPARM_CONFIG_ENTRY_CHOICE
#define SIMPARM_CONFIG_ENTRY_CHOICE

#include "ConfigEntry.hh"
#include <vector>
#include <stdexcept>

namespace simparm {

#if 0

    const std::string& operator()() const throw() { return value(); }
    const Choice& operator[](const std::string& name) const
        throw(std::invalid_argument);

    void setValue(const string &name) throw(std::invalid_argument);

    void printHelp(ostream &o) const throw();

    istream& readValue(istream& i) throw() { return (i >> value); }

    typedef std::list<ConfigNode*>::const_iterator ChoicesIterator;
    ChoicesIterator beginChoices() const throw();
    ChoicesIterator endChoices() const throw();
};
#endif

extern void formatParagraph(ostream &o, unsigned int left_col, 
                unsigned int right_col, const string &s);

template <typename ChoiceType>
class ConfigEntryTypedChoice : public ConfigEntryType<std::string> {
  public:
    typedef std::map<std::string,ChoiceType*> EntriesByName;
    typedef std::list<ChoiceType*> Entries;
  protected:
    Entries entries;
    EntriesByName entriesByName;
    GeneralConfigObject allChoices;
    bool auto_select;

    class ChoiceName : public ConfigNode {
      private:
        ConfigAttribute<std::string> &name;
        ConfigAttribute<std::string> &desc;
        std::string getName() const throw() { return name(); }
        std::string getPrefix() const throw() { return ""; }
        std::string getTypeDescriptor() const throw() {return "choice";}
      public:
        ChoiceName( ConfigObject& nameDesc ) throw()
          : name( nameDesc.name ), desc( nameDesc.desc )
          { push_back( nameDesc.name ); push_back( nameDesc.desc ); }
        ChoiceName *clone() const throw() { return new ChoiceName(*this); }
        ~ChoiceName() throw() { removeFromAllParents(); }
    };

    class ValueWatcher : public ConfigNode::Callback {
      private:
        ConfigEntryTypedChoice<ChoiceType>& papa;
        std::string lastValue;
      public:
        ValueWatcher(ConfigEntryTypedChoice<ChoiceType>& papa) throw() : papa(papa),
            lastValue(papa.value())
            { papa.value.addChangeCallback(*this); }
        ~ValueWatcher() throw() {}

        void operator()(ConfigNode& src, Cause cause, ConfigNode *) {
            if ( &src == &papa.value && cause == ValueChanged ) {
                if (papa.value() == lastValue) return;
                if ( lastValue != "")
                    papa.erase( lastValue );
                if ( papa.value() != "" ) {
                    ChoiceType *fresh = papa.get_entry( papa.value() );
                    papa.push_back( *fresh );
                }
                lastValue = papa.value();
            }
        }
    };
    ValueWatcher watcher;

    ChoiceType* get_entry( const std::string &name )
        throw(std::invalid_argument)
    {
        typename EntriesByName::iterator i = entriesByName.find( name );
        if ( i == entriesByName.end() )
            throw std::invalid_argument("There is no choice named " + name);
        else
            return i->second;
    }
    const ChoiceType* get_entry( const std::string &name ) const
        throw(std::invalid_argument)
    {
        typename EntriesByName::const_iterator i = entriesByName.find( name );
        if ( i == entriesByName.end() )
            throw std::invalid_argument("There is no choice named " + name);
        else
            return i->second;
    }

  public:
    ConfigEntryTypedChoice(string name = "", string desc = "") throw()
      : ConfigObject(name, desc),
        ConfigEntryType<std::string>(name,desc,""),
        allChoices("choices", "", "", "choices"),
        auto_select(true),
        watcher(*this)
    {
        allChoices.erase(allChoices.name);
        allChoices.erase(allChoices.desc);
        push_back(allChoices);
        push_back(value);
    }

    ConfigEntryTypedChoice(const ConfigEntryTypedChoice<ChoiceType>& o,
                           bool do_clone_entries = true)
    : ConfigObject(o), ConfigEntryType<std::string>(o.name, o.desc, ""),
      allChoices("choices", "", "", "choices"),
      auto_select(o.auto_select),
      watcher(*this)
    {
        allChoices.erase(allChoices.name);
        allChoices.erase(allChoices.desc);
        push_back(allChoices);
        push_back(value);

        if ( do_clone_entries ) {
            for (const_iterator i = o.beginChoices(); i != o.endChoices(); i++)
                addChoice( i->clone() );
            setValue( o.value() );
        }
    }

    ~ConfigEntryTypedChoice() throw() {}

    virtual ConfigEntryTypedChoice<ChoiceType>* clone() const 
        { return new ConfigEntryTypedChoice(*this); }
    std::string getTypeDescriptor() const throw()  
        { return "ConfigEntryChoice"; }

    void addChoice(ChoiceType& choice) throw() { 
        std::auto_ptr<ConfigNode> choice_name( new ChoiceName(choice) );
        allChoices.push_back( *choice_name );
        allChoices.manage( choice_name.release() );
        entries.push_back( &choice );
        entriesByName[ choice.name() ] = &choice; 
        if (auto_select && value() == "") setValue( choice.name() );
    }
    void addChoice(std::auto_ptr<ChoiceType> choice) throw() {
        this->addChoice(*choice);
        allChoices.manage( std::auto_ptr<ConfigNode>(choice) );
    }
    /** Syntactic sugar: Equivalent to the auto_ptr-call. */
    void addChoice(ChoiceType* choice) throw()
        { addChoice(std::auto_ptr<ChoiceType>(choice)); }
    void removeChoice(const string &name) throw(std::invalid_argument) {
        if (name == value()) { 
            if (auto_select && ! entries.empty() )
                setValue( entries.begin()->first );
            else
                setValue(""); 
        }
        allChoices.erase( name );
        ChoiceType *p = entriesByName[name];
        entriesByName.erase(name);
        entries.erase(p);
    }
    void removeAllChoices() { allChoices.clear(); entries.clear(); }
    
    bool isValid() const throw() { return value() != ""; }

    operator const ChoiceType&() const throw(std::out_of_range) 
        { if (isValid()) return *entriesByName.find( value() )->second;
          else throw std::out_of_range("No choice selected."); }
    const ChoiceType& getCurrentChoice() const throw(std::out_of_range) 
        { if (isValid()) return *entriesByName.find( value() )->second; 
          else throw std::out_of_range("No choice selected."); }
    ChoiceType& getCurrentChoice() throw(std::out_of_range) 
        { if (isValid()) return *entriesByName[ value() ];
          else throw std::out_of_range("No choice selected."); }
    const ChoiceType& operator[](const std::string& name) const
        throw(std::invalid_argument)
        { typename EntriesByName::iterator i = entriesByName.find( name );
          if ( i != entriesByName.end()) return *i->second;
          else throw std::invalid_argument("No choice named " + name); }
    const std::string& getValue() const throw() { return value(); }

    virtual void setValue(const string &name) { value = name; }
    void choose(const string &name) { value = name; }
    istream& readValue(istream& i) {
        std::string newVal; i >> newVal; 
        try {
            setValue(newVal);
        } catch(const std::invalid_argument&) {}
        return i;
    }

    void printHelp(ostream &o) const throw() {
        ConfigEntry::printHelp(o);
        formatParagraph(o, 0, 22, "");
        string s = "Choices are: ";
        for (ConfigNode::const_iterator i=allChoices.begin();
                i != allChoices.end(); i++)
        {
            if (value() == (*i)->getName()) s += "_";
            s += (*i)->getName();
            if (value() == (*i)->getName()) s += "_";
            s += " ";
        }
        formatParagraph(o, 23, 79, s);
        o << "\n";
    }

    /** Decide whether the first list entry should be automatically selected.
     *  If this behaviour is set to true (the default), the first entry in this
     *  choice entry will be selected automatically when no item was selected
     *  yet or the currently selected item is removed from the choice. Otherwise,
     *  the selection will be set to invalid. */
    void set_auto_selection( bool do_auto_select ) throw() { 
        auto_select = do_auto_select;
        if ( do_auto_select && !isValid() && ! entries.empty() )
            value = entries.front()->getName();
    }

    class const_iterator {
        typename Entries::const_iterator base;
      public:
        const_iterator(typename Entries::const_iterator base) throw()
            : base(base) {}

        const ChoiceType& operator*() const throw() 
            { return **base; }
        const ChoiceType* operator->() const throw() 
            { return *base; }
        const_iterator& operator++() throw() { ++base; return *this; }
        const_iterator& operator++(int) throw() { base++; return *this; }
        const_iterator& operator--() throw() { --base; return *this; }
        const_iterator& operator--(int) throw() { base--; return *this; }

        bool operator==(const const_iterator& o) throw()
            { return o.base == base; }
        bool operator!=(const const_iterator& o) throw()
            { return o.base != base; }
    };
    const_iterator beginChoices() const 
        {return const_iterator(entries.begin());}
    const_iterator endChoices() const 
        {return const_iterator(entries.end());}
};

template <typename DataType>
class DataChoice : public ConfigObject {
  private:
    DataType num;
  public:
    DataChoice(DataType num, string name = "", string desc = "") throw()
        : ConfigObject(name, desc), num(num) {}
    DataChoice<DataType>* clone() const
        { return new DataChoice<DataType>(*this); }

    operator DataType() throw() { return num; }
    const DataType& operator()() const throw() { return num; }
    DataType& operator()() throw() { return num; }
};

template <typename DataType>
class ConfigEntryDataChoice 
: public ConfigEntryTypedChoice< DataChoice<DataType> >  
{
  public:
    ConfigEntryDataChoice(string name = "", string desc = "") throw()
        : ConfigObject(name, desc),
          ConfigEntryTypedChoice<DataChoice<DataType> >(name,desc) {}
    virtual ConfigEntryDataChoice<DataType>* clone() const 
        { return new ConfigEntryDataChoice<DataType>(*this); }

    void addChoice(DataType num, string name = "", string desc = "")
    throw()
        { ConfigEntryTypedChoice< DataChoice<DataType> >::addChoice(
            new DataChoice<DataType>(num, name, desc)); }

    ConfigEntryDataChoice<DataType>&
        operator=(const DataType& choice) throw() 
    {
        for (typename ConfigEntryTypedChoice< DataChoice<DataType> >::
                const_iterator i = this->beginChoices();
                                     i != this->endChoices(); i++)
            if ( (*i)() == choice ) {
                this->setValue( i->getName() );
                break;
            }
        return *this;
    }

    const DataType& operator()() const throw() 
        { return this->getCurrentChoice()(); }
};

class ConfigEntryChoice : public ConfigEntryDataChoice<int> {
  public:
    ConfigEntryChoice(string name = "", string desc = "") throw()
      : ConfigObject(name, desc),
        ConfigEntryDataChoice<int>(name,desc) {}
    virtual ConfigEntryChoice* clone() const
        { return new ConfigEntryChoice(*this); }

    void addChoice(string name, string desc) throw() {
        this->ConfigEntryDataChoice<int>::addChoice( 
            (int)this->entries.size(), name, desc );
    }

    ConfigEntryChoice& operator=(const string& name) throw() 
        { setValue(name); return *this; }
};

}

#endif
