#ifndef SIMPARM_WX_UI_TEXTFIELDNODE_H
#define SIMPARM_WX_UI_TEXTFIELDNODE_H

#include "Node.h"
#include "AttributeHandle.h"

namespace simparm {
namespace wx_ui {

struct TextfieldNode : public Node {
    std::string description;
    std::string unit;
    boost::shared_ptr< wxWindow* > my_window;
    boost::shared_ptr< BaseAttributeHandle > value_handle;

    TextfieldNode( boost::shared_ptr<Node> n ) : Node(n) {}
    ~TextfieldNode();
    virtual void set_description( std::string d ) { description = d; }
    void initialization_finished();
    void add_attribute( simparm::BaseAttribute& a ) {
        if ( a.get_name() == "value" ) value_handle.reset( new BaseAttributeHandle(a) );
        if ( a.get_name() == "unit_symbol" ) unit = *a.get_value();
    }
};

}
}

#endif
