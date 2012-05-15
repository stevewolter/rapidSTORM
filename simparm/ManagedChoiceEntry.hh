#ifndef SIMPARM_MANAGED_CHOICE_ENTRY_HH
#define SIMPARM_MANAGED_CHOICE_ENTRY_HH

#include "ChoiceEntry.hh"
#include <boost/ptr_container/ptr_vector.hpp>

namespace simparm {

template <typename BaseClass>
class ManagedChoiceEntry 
: public ChoiceEntry<BaseClass>
{
    typedef boost::ptr_vector<BaseClass> Choices;
    Choices choices;
public:
    ManagedChoiceEntry( std::string name, std::string desc )
        : ChoiceEntry<BaseClass>(name,desc) {}
    ManagedChoiceEntry( const ManagedChoiceEntry& o )
        : ChoiceEntry<BaseClass>(o), choices(o.choices) 
    {
        for ( typename Choices::iterator i = choices.begin(); i != choices.end(); ++i )
            ChoiceEntry<BaseClass>::addChoice(*i);
    }

    void addChoice( std::auto_ptr<BaseClass> choice ) {
        ChoiceEntry<BaseClass>::addChoice(*choice);
        choices.push_back(choice);
    }
    void addChoice( BaseClass* choice ) 
        { addChoice( std::auto_ptr<BaseClass>(choice) ); }

    void removeChoice( int index ) {
        ChoiceEntry<BaseClass>::removeChoice( choices[index] );
        choices.erase( choices.begin() + index );
    }

    int size() const { return choices.size(); }

    typedef typename Choices::iterator iterator;
    typedef typename Choices::const_iterator const_iterator;
    iterator begin() { return choices.begin(); }
    iterator end() { return choices.end(); }
    const_iterator begin() const { return choices.begin(); }
    const_iterator end() const { return choices.end(); }
};

}

#endif
