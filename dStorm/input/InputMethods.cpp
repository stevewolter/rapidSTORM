#include "InputMethods.h"
#include "FileMethod.h"
#include <simparm/ChoiceEntry_Iterator.hh>
#include <simparm/ChoiceEntry_Impl.hh>

namespace dStorm {
namespace input {

InputMethods::InputMethods()
: Choice("InputMethod", "Input type", false),
  file_method( new FileMethod() )
{
    helpID = "InputType";
    userLevel = simparm::Object::Intermediate;
    add_choice(*file_method, simparm::NodeChoiceEntry<Link>::beginChoices());
}

InputMethods::InputMethods(const InputMethods& o)
: Choice(o),
  file_method( o.file_method->clone() )
{
    addChoice(simparm::NodeChoiceEntry<Link>::beginChoices(), *file_method);
    Choice::set_upstream_element( *file_method, *this, Add );
    choose( o.value().getNode().getName() );
}

InputMethods::~InputMethods()
{
    Choice::stop_receiving_changes_from(value);
    remove_choice(*file_method);
}

void InputMethods::insert_new_node( std::auto_ptr<Link> l, Place p ) 
{
    if ( p == FileReader )
        file_method->insert_new_node(l,p);
    else if ( p == InputMethod )
        Choice::insert_new_node(l,p);
    else
        throw std::logic_error("No appropriate place to insert input filter");
}

}
}
