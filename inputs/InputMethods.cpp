#include "InputMethods.h"
#include <simparm/ChoiceEntry_Iterator.hh>
#include <simparm/ChoiceEntry_Impl.hh>

namespace dStorm {
namespace input {

InputMethods::InputMethods()
: Choice("InputMethod", "Input type", false)
{
    choices.helpID = "InputType";
    choices.userLevel = simparm::Object::Intermediate;
}

InputMethods::InputMethods(const InputMethods& o)
: Choice(o)
{
}

InputMethods::~InputMethods()
{
}

void InputMethods::insert_new_node( std::auto_ptr<Link> l, Place p ) 
{
    if ( p == InputMethod )
        Choice::add_choice(l);
    else
        Choice::insert_new_node(l,p);
}

}
}
