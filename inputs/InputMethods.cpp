#include "InputMethods.h"
#include <simparm/ChoiceEntry_Iterator.hh>
#include <simparm/ChoiceEntry_Impl.hh>
#include <dStorm/input/chain/Choice.h>

namespace dStorm {
namespace inputs {
namespace InputMethods {

struct Link 
: public input::chain::Choice 
{
    Link();
    Link* clone() const { return new Link(*this); }
    void insert_new_node( std::auto_ptr<input::chain::Link>, Place );
};

Link::Link()
: Choice("InputMethod", "Input type", false)
{
    choices.helpID = "InputType";
    choices.userLevel = simparm::Object::Intermediate;
}

void Link::insert_new_node( std::auto_ptr<input::chain::Link> l, Place p ) 
{
    if ( p == InputMethod )
        Choice::add_choice(l);
    else
        Choice::insert_new_node(l,p);
}

std::auto_ptr< input::chain::Link > create()
    { return std::auto_ptr< input::chain::Link >(new Link()); }

}
}
}
