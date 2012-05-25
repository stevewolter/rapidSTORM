#include "BasicEntry.h"
#include "Node.h"

namespace simparm {

using std::string;

BasicEntry::BasicEntry(string name, string desc)
: Object(name, desc),
  help("help", ""),
  invalid("invalid", false),
  editable("editable", true),
  outputOnChange("outputOnChange", true),
  helpID("helpID", "")
{
}

BasicEntry::BasicEntry(const BasicEntry& from)
:   Object(from),
    help(from.help),
    invalid(from.invalid),
    editable(from.editable),
    outputOnChange(from.outputOnChange),
    helpID(from.helpID)
{
}

NodeHandle BasicEntry::create_hidden_node( simparm::NodeHandle node ) {
    NodeHandle r = Object::create_hidden_node( node );
    r->add_attribute(help);
    r->add_attribute(invalid);
    r->add_attribute(editable);
    r->add_attribute(outputOnChange);
    r->add_attribute(helpID);
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

}
