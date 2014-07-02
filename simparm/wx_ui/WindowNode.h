#ifndef SIMPARM_WX_UI_WINDOWNODE_H
#define SIMPARM_WX_UI_WINDOWNODE_H

#include "simparm/wx_ui/Node.h"
#include "simparm/wx_ui/GUIHandle.h"
#include "simparm/wx_ui/Sizer.h"

namespace simparm {
namespace wx_ui {

class WindowNode : public InnerNode {
    Sizer sizer;
    WindowSpecification window;
    virtual boost::shared_ptr<Window> create_window();
  protected:
    void set_self_growing() { window.proportion = 1; }
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

class TopWindowNode : public WindowNode {
    boost::shared_ptr<bool> needs_fit_inside;

  public:
    TopWindowNode( boost::shared_ptr<Node> n, std::string name )
        : WindowNode(n, name), needs_fit_inside(new bool(false)) {}
    boost::function0<void> get_relayout_function();
};

}
}

#endif
