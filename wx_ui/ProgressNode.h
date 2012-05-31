#ifndef SIMPARM_WX_UI_PROGRESSNODE_H
#define SIMPARM_WX_UI_PROGRESSNODE_H

#include "Node.h"

class wxGauge;

namespace simparm {
namespace wx_ui {

struct ProgressNode : public Node {
    std::string description;
    boost::shared_ptr< wxGauge* > my_gauge;
    simparm::BaseAttribute* value;
    BaseAttribute::ConnectionStore connection;

    ProgressNode( boost::shared_ptr<Node> n ) 
        : Node(n), my_gauge( new wxGauge*() ), value(NULL) {}
    virtual void set_description( std::string d ) { description = d; }
    void initialization_finished();
    void display_value();
    void add_attribute( simparm::BaseAttribute& a ) {
        if ( a.get_name() == "value" )  {
            value = &a;
            connection = a.notify_on_non_GUI_value_change( boost::bind( &ProgressNode::display_value, this ) );
        }
    }
};

}
}

#endif

