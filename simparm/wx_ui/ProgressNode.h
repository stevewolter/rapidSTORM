#ifndef SIMPARM_WX_UI_PROGRESSNODE_H
#define SIMPARM_WX_UI_PROGRESSNODE_H

#include "Node.h"
#include <wx/gauge.h>

namespace simparm {
namespace wx_ui {

class ProgressNode : public InnerNode {
    std::string description;
    boost::shared_ptr< wxGauge* > my_gauge;
    boost::optional<LineSpecification> my_line;
    simparm::BaseAttribute* value, *indeterminate;
    BaseAttribute::ConnectionStore connection[2];
    bool determinate;

public:
    ProgressNode( boost::shared_ptr<Node> n, std::string name ) 
        : InnerNode(n,name), my_gauge( new wxGauge*() ), value(NULL), determinate(true) {}
    virtual void set_description( std::string d ) { description = d; }
    void initialization_finished();
    void display_value();
    void set_determinate_mode();
    void add_attribute( simparm::BaseAttribute& a ) {
        if ( a.get_name() == "value" )  {
            value = &a;
            connection[0] = a.notify_on_non_GUI_value_change( boost::bind( &ProgressNode::display_value, this ) );
        }
        if ( a.get_name() == "indeterminate" )  {
            indeterminate = &a;
            connection[1] = a.notify_on_non_GUI_value_change( boost::bind( &ProgressNode::set_determinate_mode, this ) );
        }
    }
};

}
}

#endif

