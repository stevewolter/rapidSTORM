#ifndef SIMPARM_WX_UI_TABNODE_H
#define SIMPARM_WX_UI_TABNODE_H

#include "WindowNode.h"

class wxNotebook;

namespace simparm {
namespace wx_ui {

class TabNode : public WindowNode {
    boost::shared_ptr<wxNotebook*> notebook;

public:
    TabNode( boost::shared_ptr<Node> n )
        : WindowNode(n), notebook( new wxNotebook*() ) {}
    virtual void set_description( std::string d ) { window.name = d; }
    void initialization_finished();
    void add_entry_line( const LineSpecification& );
    void add_full_width_line( WindowSpecification w );
    NodeHandle create_object( std::string name );
    NodeHandle create_group( std::string name );
    boost::function0<void> get_relayout_function();
};

}
}

#endif
