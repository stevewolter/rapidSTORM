#ifndef SIMPARM_WX_UI_TREEREPRESENTATION_H
#define SIMPARM_WX_UI_TREEREPRESENTATION_H

#include <wx/string.h>
#include <wx/treebook.h>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/weak_ptr.hpp>
#include <vector>
#include "simparm/wx_ui/Node.h"

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

    void add_as_child( boost::shared_ptr< TreeRepresentation > parent, boost::shared_ptr<Window> window, wxString name, boost::function0<void> redraw );
    void remove_child( boost::shared_ptr<Window> window );
    void create_widget(boost::shared_ptr<Window> window_announcement, boost::shared_ptr<Window> parent );
    void InvalidateBestSize() { widget->InvalidateBestSize(); }
};

}
}

#endif
