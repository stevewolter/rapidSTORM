#ifndef SIMPARM_CMDLINE_UI_PROGRESS_NODE_H
#define SIMPARM_CMDLINE_UI_PROGRESS_NODE_H

#include "Node.h"

namespace simparm {
namespace cmdline_ui {

class ProgressNode : public Node {
#if 0
    boost::ptr_vector< boost::signals2::scoped_connection > connections;
    virtual void add_attribute( simparm::BaseAttribute& );
    std::string desc;
    double last_value;

    void set_description();
    void set_value();
#endif
public:
    ProgressNode( std::string name ) : Node(name) {}
};

}
}

#endif
