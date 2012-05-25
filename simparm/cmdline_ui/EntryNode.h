#ifndef SIMPARM_CMDLINE_UI_ENTRYNODE_H
#define SIMPARM_CMDLINE_UI_ENTRYNODE_H

#include "Node.h"
#include "OptionTable.h"

namespace simparm {
namespace cmdline_ui {

class EntryNode : public Node
{
    std::string name, desc, choices;
    BaseAttribute* value, *help;
    OptionTable::Type type;

protected:
    virtual void program_options( OptionTable& t ) {
        if ( this->is_visible() ) {
            std::string help = "";
            if ( this->help ) {
                std::string help_decl = this->help->get_value();
                if ( help_decl.substr(0,4) == "set " )
                    help = help_decl.substr(4);
            }
            t.add_option( name, get_description(), help, choices, *value, type );
        }
    }
    virtual void add_attribute( simparm::BaseAttribute& a ) {
        if ( a.get_name() == "value" ) value = &a;
        if ( a.get_name() == "help" ) help = &a;
    }
    void set_choices( std::string choices ) { this->choices = choices; }
public:
    EntryNode( std::string name, OptionTable::Type type ) 
        : Node(name), name(name), value(NULL), help(NULL), type(type) {}
};

}
}

#endif
