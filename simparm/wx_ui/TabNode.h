#ifndef SIMPARM_WX_UI_TABNODE_H
#define SIMPARM_WX_UI_TABNODE_H

#include "simparm/wx_ui/WindowNode.h"
#include "simparm/wx_ui/GUIHandle.h"

namespace simparm {
namespace wx_ui {

class Notebook;

class TabNode : public InnerNode {
    boost::shared_ptr<bool> notebook_valid_;
    WindowSpecification window;
    GUIHandle<Notebook> notebook;

public:
    TabNode( boost::shared_ptr<Node> n, std::string name ) : InnerNode(n, name), notebook_valid_(new bool(true)) {}
    virtual void set_description( std::string d ) { window.name = d; }
    void initialization_finished();
    void add_entry_line( LineSpecification& );
    void add_full_width_line( WindowSpecification& w );
    void add_full_width_sizer( SizerSpecification& w );
    NodeHandle create_object( std::string name );
    NodeHandle create_group( std::string name );
    boost::function0<void> get_relayout_function();
    boost::shared_ptr< Window > get_parent_window() { return window.window; }
};

}
}

#endif
