#include "InputMethods.h"
#include "FileMethod.h"
#include <simparm/ChoiceEntry_Iterator.hh>
#include <simparm/ChoiceEntry_Impl.hh>

namespace dStorm {
namespace input {

InputMethods::InputMethods()
: Choice("InputMethod", "Input type", false)
{
    choices.helpID = "InputType";
    choices.userLevel = simparm::Object::Intermediate;
    add_choice( std::auto_ptr<Link>(new FileMethod()) );
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
    if ( p == FileReader )
        Choice::get_first_link().insert_new_node(l,p);
    else if ( p == InputMethod )
        Choice::insert_new_node(l,p);
    else
        throw std::logic_error("No appropriate place to insert input filter");
}

}
}
