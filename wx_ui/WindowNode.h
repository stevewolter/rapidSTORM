#ifndef SIMPARM_WX_UI_WINDOWNODE_H
#define SIMPARM_WX_UI_WINDOWNODE_H

#include "Node.h"

class wxGridBagSizer;

namespace simparm {
namespace wx_ui {

class WindowNode : public Node {
protected:
    WindowSpecification window;
    boost::shared_ptr<wxGridBagSizer*> sizer;
    boost::shared_ptr<int> row;

    void create_unattached_gridbag_sizer();
    void create_sizer();

public:
    WindowNode( boost::shared_ptr<Node> n ) ;
    virtual void set_description( std::string d ) { window.name = d; }
    void initialization_finished();

    void add_entry_line( const LineSpecification& );
    void add_full_width_line( WindowSpecification w );
    void add_full_width_sizer( SizerSpecification w );
    boost::shared_ptr< wxWindow* > get_parent_window() { return window.window; }
};

}
}

#endif
