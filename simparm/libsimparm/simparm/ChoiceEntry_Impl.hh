#ifndef SIMPARM_CHOICEENTRY_IMPL_HH
#define SIMPARM_CHOICEENTRY_IMPL_HH

#include "ChoiceEntry.hh"

namespace simparm {
    template <typename ChoiceType>
    NodeChoiceEntry<ChoiceType>::
    NodeChoiceEntry(string name, string desc)
      : Entry(name,desc),
        auto_select(true),
        choiceChecker(entries),
        value( "value", *this )
    {
        value.change_is_OK = &choiceChecker;
        push_back( value );
    }


    template <typename ChoiceType>
    NodeChoiceEntry<ChoiceType>::
    NodeChoiceEntry(const NodeChoiceEntry<ChoiceType>& o,
                    EntryDuplicationPolicy duplicationPolicy)
    : Node(o), Entry(o), Attribute<ChoiceType*>::NameMap(),
      auto_select(o.auto_select),
      choiceChecker(entries),
      value("value", *this)
    {
        value.change_is_OK = &choiceChecker;
        push_back( value );
        if ( duplicationPolicy == DeepCopy ) {
            for (const_iterator i = o.beginChoices(); i != o.endChoices(); i++) {
                std::auto_ptr<ChoiceType> clone( i->clone() );
                addChoice( *clone );
                if ( &*i == &o.value() )
                    setValue( *clone );
                this->manage( std::auto_ptr<Node>( clone ) );
            }
        } else if ( duplicationPolicy == ShallowCopy ) {
            for (const_iterator i = o.beginChoices(); i != o.endChoices(); i++)
                addChoice( const_cast<ChoiceType&>(*i) );
            this->value = const_cast<ChoiceType&>( o.value() );
        }
    }

    template <typename ChoiceType>
    NodeChoiceEntry<ChoiceType>::~NodeChoiceEntry() {
    }

    template <typename ChoiceType>
    void NodeChoiceEntry<ChoiceType>::
    addChoice(ChoiceType& choice)
    { 
        push_back( choice );
        entries.insert( &choice );
        std::string name = choice.getName();
        entriesByName.insert( make_pair( choice.getName(), &choice ) );
        if (auto_select && !value.hasValue()) setValue( &choice );
    }

    template <typename ChoiceType>
    void NodeChoiceEntry<ChoiceType>::
    addChoice(std::auto_ptr<ChoiceType> choice) {
        this->addChoice(*choice);
        this->manage( std::auto_ptr<Node>(choice) );
    }

    template <typename ChoiceType>
    void NodeChoiceEntry<ChoiceType>::
    removeChoice(ChoiceType &choice)
    {
        bool auto_select_new_value = false;
        if (&choice == &value()) { 
            auto_select_new_value = this->auto_select;
            setValue(NULL); 
        }
        ChoiceType *p = &choice;
        entriesByName.erase( choice.getName() );
        entries.erase( p );
        if ( auto_select_new_value && !entries.empty() )
            setValue( *entries.begin() );
        this->Node::erase( *p );
    }

    template <typename ChoiceType>
    void NodeChoiceEntry<ChoiceType>::removeAllChoices()
    { 
        for ( typename Entries::iterator i = entries.begin(); 
              i != entries.end(); i++)
        {
            this->Node::erase( **i );
            if ( &value() == *i ) value = NULL;
        }
        entriesByName.clear();
        entries.clear();
    }
    
    template <typename ChoiceType>
    std::list<std::string> 
    NodeChoiceEntry<ChoiceType>::printValues() const
    {
        std::list<std::string> children = Object::printValues();
        children.push_front( getName() + " = " + value.getValueString() );
        return children;
    }
    
    template <typename ChoiceType>
    void NodeChoiceEntry<ChoiceType>::
        printHelp(ostream &o) const 
    {
        Entry::printHelp(o);
        formatParagraph(o, 0, 22, "");
        string s = "Choices are: ";
        for (typename Entries::const_iterator i= entries.begin();
                i !=  entries.end(); i++)
        {
            if (&value() == (*i)) s += "_";
            s += (*i)->getName();
            if (&value() == (*i)) s += "_";
            s += " ";
        }
        formatParagraph(o, 23, 79, s);
        o << "\n";
    }

    template <typename DataType>
    DataChoiceEntry<DataType>::
    DataChoiceEntry(string name, string desc)
        : NodeChoiceEntry<DataChoice<DataType> >(name,desc) {}

    template <typename DataType>
    DataChoiceEntry<DataType>::
    ~DataChoiceEntry() {}

    template <typename DataType>
    DataChoiceEntry<DataType>* 
        DataChoiceEntry<DataType>::clone() const
        { return new DataChoiceEntry<DataType>(*this); }

    template <typename DataType>
    void DataChoiceEntry<DataType>::
    addChoice(DataType num, string name, string desc)
        { NodeChoiceEntry< DataChoice<DataType> >::addChoice(
            new DataChoice<DataType>(num, name, desc)); }

    ChoiceEntry::ChoiceEntry(string name, string desc)
      : DataChoiceEntry<int>(name,desc) {}

    ChoiceEntry::~ChoiceEntry() {}

    ChoiceEntry* ChoiceEntry::clone() const
        { return new ChoiceEntry(*this); }

    void ChoiceEntry::
    addChoice(int pos, string name, string desc)
    {
        this->DataChoiceEntry<int>::addChoice( pos, name, desc );
    }

    void ChoiceEntry::
    addChoice(string name, string desc)
    {
        this->DataChoiceEntry<int>::addChoice( 
            (int)this->entries.size(), name, desc );
    }
}

#endif
