#ifndef SIMPARM_WX_UI_SCROLLEDWINDOWNODE_H
#define SIMPARM_WX_UI_SCROLLEDWINDOWNODE_H

#include "WindowNode.h"

class wxScrolledWindow;

namespace simparm {
namespace wx_ui {

class ScrolledWindowNode : public WindowNode {
    boost::shared_ptr< wxScrolledWindow* > scrolled_window;
public:
    ScrolledWindowNode( boost::shared_ptr<Node> n ) 
        : WindowNode(n), scrolled_window( new wxScrolledWindow*() ) {}
    void initialization_finished();
    boost::function0<void> get_relayout_function() ;
};

}
}

#endif
