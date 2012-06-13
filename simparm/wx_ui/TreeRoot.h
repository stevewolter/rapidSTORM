#ifndef SIMPARM_WX_UI_TREE_ROOT_H
#define SIMPARM_WX_UI_TREE_ROOT_H

#include "Node.h"

namespace simparm {
namespace wx_ui {

class TreeRoot : public InnerNode {
    boost::shared_ptr< Window > treebook_widget;
    boost::shared_ptr< TreeRepresentation > tr_root;

public:
    TreeRoot( boost::shared_ptr<Node> n, std::string name ) : InnerNode(n, name) {}
    boost::shared_ptr< TreeRepresentation > get_treebook_parent() { return tr_root; }
    boost::shared_ptr< Window > get_treebook_widget() { return treebook_widget; }
    boost::function0<void> get_relayout_function();
    void initialization_finished();
};

}
}

#endif
