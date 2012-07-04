#ifndef SIMPARM_WX_UI_WINDOWNODE_H
#define SIMPARM_WX_UI_WINDOWNODE_H

#include "Node.h"
#include "GUIHandle.h"
#include "Sizer.h"

namespace simparm {
namespace wx_ui {

class WindowNode : public InnerNode {
    Sizer sizer;
    WindowSpecification window;
    virtual boost::shared_ptr<Window> create_window();
public:
    WindowNode( boost::shared_ptr<Node> n, std::string name ) ;
    virtual void set_description( std::string d ) { window.name = d; }
    void initialization_finished();
    boost::function0<void> get_relayout_function();

    void add_entry_line( LineSpecification& l ) { sizer.add_entry_line(l); }
    void add_full_width_line( WindowSpecification& w ) { sizer.add_full_width_line(w); }
    void add_full_width_sizer( SizerSpecification& w ) { sizer.add_full_width_sizer(w); }
    boost::shared_ptr< Window > get_parent_window() { return window.window; }
};

}
}

#endif
