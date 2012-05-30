#ifndef SIMPARM_WX_UI_CHOICENODE_H
#define SIMPARM_WX_UI_CHOICENODE_H

#include "Node.h"

namespace simparm {
namespace wx_ui {

class ChoiceWidget;

class ChoiceNode : public Node {
    class SubNode;
    LineSpecification choice_line;
    boost::shared_ptr<ChoiceWidget*> choice;
    std::string description;
    simparm::BaseAttribute::ConnectionStore connection;

    simparm::BaseAttribute* value;

public:
    ChoiceNode( boost::shared_ptr<Node> n )
        : Node(n), choice( new ChoiceWidget*() ) {}
    virtual void set_description( std::string d ) { description = d; }
    void initialization_finished();
    NodeHandle create_object( std::string name );
    NodeHandle create_group( std::string name );
    void add_attribute( simparm::BaseAttribute& a ) {
        if ( a.get_name() == "value" ) value = &a;
    }
};

}
}

#endif
