#ifndef SIMPARM_CMDLINE_UI_ENTRYNODE_H
#define SIMPARM_CMDLINE_UI_ENTRYNODE_H

#include "simparm/cmdline_ui/Node.h"
#include "simparm/cmdline_ui/OptionTable.h"

namespace simparm {
namespace cmdline_ui {

class EntryNode : public Node
{
    std::string name, choices, help;
    BaseAttribute* value;
    OptionTable::Type type;

protected:
    virtual void program_options( OptionTable& t ) {
        if ( this->is_visible() ) {
            t.add_option( name, get_description(), help, choices, *value, type );
        }
    }
    virtual void add_attribute( simparm::BaseAttribute& a ) {
        if ( a.get_name() == "value" ) value = &a;
    }
    void set_choices( std::string choices ) { this->choices = choices; }
    void set_help( std::string h ) { help = h; }
public:
    EntryNode( std::string name, OptionTable::Type type ) 
        : Node(name), name(name), value(NULL), type(type) {}
};

}
}

#endif
