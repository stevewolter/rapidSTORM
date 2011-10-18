#include "InputMethods.h"
#include "FileMethod.h"
#include "FileInput.h"
#include <simparm/ChoiceEntry_Iterator.hh>
#include <simparm/ChoiceEntry_Impl.hh>

namespace dStorm {
namespace input {

InputMethods::InputMethods()
: Choice("InputMethod", "Input type", false),
  file_method( new FileMethod() )
{
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

void InputMethods::insert_new_node( std::auto_ptr<Link> l ) 
{
    if ( dynamic_cast< FileInput* >(l.get()) )
        file_method->insert_new_node(l);
    else
        Choice::insert_new_node(l);
}

}
}
