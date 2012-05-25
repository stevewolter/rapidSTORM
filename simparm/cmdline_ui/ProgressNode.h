#ifndef SIMPARM_CMDLINE_UI_PROGRESS_NODE_H
#define SIMPARM_CMDLINE_UI_PROGRESS_NODE_H

#include "Node.h"
#include "../Attribute.h"

namespace simparm {
namespace cmdline_ui {

class ProgressNode : public Node {
    simparm::Attribute<double>* value;
    std::auto_ptr<boost::signals2::scoped_connection> connections;
    void add_attribute( simparm::BaseAttribute& );

    int last_percentage;

    void set_value();
public:
    ProgressNode( std::string name ) : Node(name), value(NULL), last_percentage(0) {}
};

}
}

#endif
