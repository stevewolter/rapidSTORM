#ifndef SIMPARM_WX_UI_TEXTFIELDNODE_H
#define SIMPARM_WX_UI_TEXTFIELDNODE_H

#include "Node.h"
#include "AttributeHandle.h"

namespace simparm {
namespace wx_ui {

class TextCtrl;

class TextfieldNode : public InnerNode {
public:
    enum Mode { Value, File };

    TextfieldNode( boost::shared_ptr<Node> n, std::string name, Mode mode = Value ) 
        : InnerNode(n, name), my_window( new TextCtrl*() ), rows(1), columns(1), mode(mode) {}
    ~TextfieldNode();
    virtual void set_description( std::string d ) { description = d; }
    void initialization_finished();
    void add_attribute( simparm::BaseAttribute& a );
private:
    std::string description;
    std::string unit;
    boost::shared_ptr< TextCtrl* > my_window;
    boost::shared_ptr< BaseAttributeHandle > value_handle;
    BaseAttribute::ConnectionStore connection;

    int rows, columns;
    Mode mode;

    void display_value();
};

}
}

#endif
