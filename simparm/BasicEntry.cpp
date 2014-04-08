#include "simparm/BasicEntry.h"
#include "simparm/Node.h"
#include "simparm/GUILabelTable.h"

namespace simparm {

using std::string;

BasicEntry::BasicEntry(string name, string desc)
: Object(name, desc), editable(true)
{
}

BasicEntry::BasicEntry(string name)
: Object(name), editable(true)
{
    const GUILabelTable::Entry& e = GUILabelTable::get_singleton().get_entry( name );
    help = e.help;
    helpID = e.helpID;
}


BasicEntry::BasicEntry(const BasicEntry& from)
:   Object(from), help(from.help), helpID(from.helpID), editable(from.editable)
{
}

NodeHandle BasicEntry::create_hidden_node( simparm::NodeHandle node ) {
    NodeHandle r = Object::create_hidden_node( node );
    r->set_help( help ); 
    r->set_editability(editable); 
    r->set_help_id( helpID ); 
    return r;
}

BasicEntry::~BasicEntry() 
{}

NodeHandle BasicEntry::create_textfield( NodeHandle parent, std::string name, std::string type ) {
    return parent->create_textfield( name, type );
}
NodeHandle BasicEntry::create_checkbox( NodeHandle parent, std::string name ) {
    return parent->create_checkbox( name );
}
NodeHandle BasicEntry::create_choice( NodeHandle parent, std::string name ) {
    return parent->create_choice( name );
}

void BasicEntry::setHelp(const std::string &help) { 
    this->help = help;
    get_user_interface_handle()->set_help( help ); 
}
void BasicEntry::setEditable(bool editable) { 
    this->editable = editable;
    get_user_interface_handle()->set_editability(editable); 
}
void BasicEntry::setHelpID( const std::string &helpID) { 
    this->helpID = helpID;
    get_user_interface_handle()->set_help_id( helpID ); 
}

}
