#include "InputMethods.h"
#include <simparm/ChoiceEntry_Impl.h>
#include <dStorm/input/Choice.h>

namespace dStorm {
namespace inputs {
namespace InputMethods {

struct Link 
: public input::Choice 
{
    Link();
    Link* clone() const { return new Link(*this); }
    void insert_new_node( std::auto_ptr<input::Link>, Place );
};

Link::Link()
: Choice("InputMethod", "Input type", false)
{
    choices.helpID = "InputType";
    choices.set_user_level( simparm::Intermediate );
}

void Link::insert_new_node( std::auto_ptr<input::Link> l, Place p ) 
{
    if ( p == InputMethod )
        Choice::add_choice(l);
    else
        Choice::insert_new_node(l,p);
}

std::auto_ptr< input::Link > create()
    { return std::auto_ptr< input::Link >(new Link()); }

}
}
}
