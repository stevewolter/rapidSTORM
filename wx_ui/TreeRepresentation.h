#ifndef SIMPARM_WX_UI_TREEREPRESENTATION_H
#define SIMPARM_WX_UI_TREEREPRESENTATION_H

#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/weak_ptr.hpp>
#include <vector>
#include "Node.h"

class wxTreebook;
class wxWindow;

namespace simparm {
namespace wx_ui {

class TreeRepresentation {
    wxTreebook* widget;
    boost::shared_ptr< TreeRepresentation > parent;
    std::vector< TreeRepresentation* > children;

    int get_preceding_and_self_count() const;
    int node_count() const;

public:
    TreeRepresentation();

    void add_as_child( boost::shared_ptr< TreeRepresentation > parent, const WindowSpecification& page, boost::function0<void> redraw );
    void create_widget(boost::shared_ptr<wxWindow*> window_announcement, boost::shared_ptr<wxWindow*> parent );
};

}
}

#endif
