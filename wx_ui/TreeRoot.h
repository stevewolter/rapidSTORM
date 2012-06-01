#ifndef SIMPARM_WX_UI_TREE_ROOT_H
#define SIMPARM_WX_UI_TREE_ROOT_H

#include "Node.h"

namespace simparm {
namespace wx_ui {

class TreeRoot : public Node {
    boost::shared_ptr< wxWindow* > treebook_widget;
    boost::shared_ptr< TreeRepresentation > tr_root;

public:
    TreeRoot( boost::shared_ptr<Node> n ) : Node(n) {}
    boost::shared_ptr< TreeRepresentation > get_treebook_parent() { return tr_root; }
    boost::shared_ptr< wxWindow* > get_treebook_widget() { return treebook_widget; }
    void initialization_finished();
};

}
}

#endif
