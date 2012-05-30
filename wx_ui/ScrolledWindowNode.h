#ifndef SIMPARM_WX_UI_SCROLLEDWINDOWNODE_H
#define SIMPARM_WX_UI_SCROLLEDWINDOWNODE_H

#include "WindowNode.h"

namespace simparm {
namespace wx_ui {

class ScrolledWindowNode : public WindowNode {
public:
    ScrolledWindowNode( boost::shared_ptr<Node> n ) 
        : WindowNode(n) {}
    void initialization_finished();
};

}
}

#endif
