#include "BasicEntry.h"
#include "Node.h"

namespace simparm {

using std::string;

BasicEntry::BasicEntry(string name, string desc)
: Object(name, desc)
{
}

BasicEntry::BasicEntry(const BasicEntry& from)
:   Object(from)
{
}

NodeHandle BasicEntry::create_hidden_node( simparm::NodeHandle node ) {
    NodeHandle r = Object::create_hidden_node( node );
    return r;
}

BasicEntry::~BasicEntry() 
{}

NodeHandle BasicEntry::create_textfield( NodeHandle parent, std::string name, std::string type ) {
    return parent->create_entry( name, type );
}
NodeHandle BasicEntry::create_checkbox( NodeHandle parent, std::string name ) {
    return parent->create_entry( name, "Bool" );
}
NodeHandle BasicEntry::create_choice( NodeHandle parent, std::string name ) {
    return parent->create_choice( name );
}

void BasicEntry::setHelp(const std::string &help) { get_user_interface_handle()->set_help( help ); }
void BasicEntry::setEditable(bool editable) { get_user_interface_handle()->set_editability(editable); }
void BasicEntry::setHelpID( const std::string &helpID) { get_user_interface_handle()->set_help_id( helpID ); }

}
