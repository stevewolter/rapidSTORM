#ifndef SIMPARM_WX_UI_SCROLLEDTABNODE_H
#define SIMPARM_WX_UI_SCROLLEDTABNODE_H

#include "Node.h"
#include "GUIHandle.h"
#include "ScrolledWindowNode.h"
#include <wx/scrolwin.h>
#include <wx/sizer.h>

namespace simparm {
namespace wx_ui {

class AUINotebook;

struct ScrolledTabNode : public InnerNode {
    WindowSpecification window;
    GUIHandle<AUINotebook> notebook;

public:
    ScrolledTabNode( boost::shared_ptr<Node> n, std::string name ) : InnerNode(n, name) {}
    virtual void set_description( std::string d ) { window.name = d; }
    void initialization_finished();
    void add_entry_line( LineSpecification& );
    void add_full_width_line( WindowSpecification& w );
    void add_full_width_sizer( SizerSpecification& w );
    NodeHandle create_object( std::string name );
    NodeHandle create_group( std::string name );
    boost::shared_ptr< Window > get_parent_window() { return window.window; }

    /** Store the current tab in a file. This method must be called by the GUI thread. */
    void serialize_current_tab( std::string filename );
    void close_all_tabs();
};


}
}

#endif
