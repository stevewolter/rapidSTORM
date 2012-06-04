#ifndef SIMPARM_WX_UI_TREENODE_H
#define SIMPARM_WX_UI_TREENODE_H

#include "Node.h"

class TreeRepresentation;

namespace simparm {
namespace wx_ui {

class TreePage : public InnerNode {
    boost::shared_ptr< TreeRepresentation > tr_node;
public:
    TreePage( boost::shared_ptr<Node> n ) : InnerNode(n) {}

    void add_entry_line( LineSpecification& s );
    void add_full_width_line( WindowSpecification& w );
    void add_full_width_sizer( SizerSpecification& );
    boost::shared_ptr< TreeRepresentation > get_treebook_parent() { return tr_node; }
    boost::shared_ptr< Window > get_parent_window() { return get_treebook_widget(); }
};

}
}

#endif

