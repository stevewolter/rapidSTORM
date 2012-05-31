#ifndef SIMPARM_WX_UI_CHOICENODE_H
#define SIMPARM_WX_UI_CHOICENODE_H

#include "Node.h"
#include "AttributeHandle.h"

namespace simparm {
namespace wx_ui {

class ChoiceWidget;

class ChoiceNode : public Node {
    class SubNode;
    boost::shared_ptr<ChoiceWidget*> choice;
    std::string description;
    simparm::BaseAttribute::ConnectionStore connection;

    boost::shared_ptr< BaseAttributeHandle > value_handle;
    void user_changed_choice();

public:
    ChoiceNode( boost::shared_ptr<Node> n )
        : Node(n), choice( new ChoiceWidget*() ) {}
    virtual void set_description( std::string d ) { description = d; }
    void initialization_finished();
    NodeHandle create_object( std::string name );
    NodeHandle create_group( std::string name );
    void add_attribute( simparm::BaseAttribute& a );
};

}
}

#endif
