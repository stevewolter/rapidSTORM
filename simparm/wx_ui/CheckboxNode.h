#ifndef SIMPARM_WX_UI_CHECKBOXNODE_H
#define SIMPARM_WX_UI_CHECKBOXNODE_H

#include "Node.h"
#include "AttributeHandle.h"

namespace simparm {
namespace wx_ui {

class CheckBox;

class CheckboxNode : public InnerNode {
    std::string description;
    std::string unit;
    boost::shared_ptr< CheckBox* > my_window;
    boost::shared_ptr< AttributeHandle<bool> > value_handle;
    BaseAttribute::ConnectionStore connection;
    void display_value();

public:
    CheckboxNode( boost::shared_ptr<Node> n, std::string name ) : InnerNode(n,name), my_window( new CheckBox*() ) {}
    ~CheckboxNode();
    virtual void set_description( std::string d ) { description = d; }
    void initialization_finished();
    void add_attribute( simparm::BaseAttribute& a );
};

}
}

#endif
