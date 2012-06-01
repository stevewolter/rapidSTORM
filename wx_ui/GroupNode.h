#ifndef SIMPARM_WX_UI_GROUPNODE_H
#define SIMPARM_WX_UI_GROUPNODE_H

#include "WindowNode.h"

class wxSizer;

namespace simparm {
namespace wx_ui {

class GroupNode : public WindowNode {
    boost::shared_ptr< wxSizer* > box_sizer;
    std::string description;

public:
    GroupNode( boost::shared_ptr<Node> n )
        : WindowNode(n), box_sizer( new wxSizer*() ) {}
    virtual void set_description( std::string d ) { window.name = d; description = d; }
    void initialization_finished();
    boost::shared_ptr< wxWindow* > get_parent_window() { return Node::get_parent_window(); }
};

}
}

#endif
