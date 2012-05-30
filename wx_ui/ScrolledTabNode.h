#ifndef SIMPARM_WX_UI_SCROLLEDTABNODE_H
#define SIMPARM_WX_UI_SCROLLEDTABNODE_H

#include "TabNode.h"
#include "ScrolledWindowNode.h"
#include <wx/scrolwin.h>
#include <wx/sizer.h>

namespace simparm {
namespace wx_ui {

struct ScrolledTabNode : public TabNode {
    ScrolledTabNode( boost::shared_ptr<Node> n ) : TabNode(n) {}

    NodeHandle create_object( std::string ) {
        return NodeHandle( new ScrolledWindowNode( shared_from_this() ) );
    }

    NodeHandle create_group( std::string ) {
        return NodeHandle( new ScrolledWindowNode( shared_from_this() ) );
    }

};


}
}

#endif
