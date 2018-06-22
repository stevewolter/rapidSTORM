#ifndef SIMPARM_WX_UI_GROUPNODE_H
#define SIMPARM_WX_UI_GROUPNODE_H

#include "simparm/wx_ui/Node.h"
#include "simparm/wx_ui/GUIHandle.h"
#include "simparm/wx_ui/Sizer.h"

class wxSizer;

namespace simparm {
namespace wx_ui {

class GroupNode : public InnerNode {
    Sizer sizer;
    GUIHandle< wxSizer > box_sizer;
    std::string description;

public:
    GroupNode( boost::shared_ptr<Node> n, std::string name ) : InnerNode(n, name) {}
    virtual void set_description( std::string d ) { description = d; }
    void add_entry_line( LineSpecification& l );
    void add_full_width_line( WindowSpecification& w );
    void add_full_width_sizer( SizerSpecification& w ) { sizer.add_full_width_sizer(w); }
    void initialization_finished();
};

}
}

#endif
