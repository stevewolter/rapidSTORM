#ifndef SIMPARM_CMDLINE_UI_TRIGGERNODE_H
#define SIMPARM_CMDLINE_UI_TRIGGERNODE_H

#include "Node.h"
#include "OptionTable.h"

namespace simparm {
namespace cmdline_ui {

struct TriggerNode : public Node
{
    std::string name;
    BaseAttribute* value;
    virtual void program_options( OptionTable& t ) {
        t.add_option( name, *value, OptionTable::Trigger );
    }
    virtual void add_attribute( simparm::BaseAttribute& a ) {
        if ( a.get_name() == "value" ) value = &a;
    }
public:
    TriggerNode( std::string name ) : Node(name), name(name), value(NULL) {}
};

}
}

#endif
