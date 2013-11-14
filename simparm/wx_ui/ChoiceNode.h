#ifndef SIMPARM_WX_UI_CHOICENODE_H
#define SIMPARM_WX_UI_CHOICENODE_H

#include "AttributeHandle.h"
#include "Node.h"

namespace simparm {
namespace wx_ui {

class ChoiceWidget;

class ChoiceNode : public InnerNode {
    template <typename T> class SubNode;
    boost::optional<LineSpecification> my_line;
    boost::shared_ptr<ChoiceWidget*> choice;
    std::string description;
    simparm::BaseAttribute::ConnectionStore connection;
    bool visible;
    int choices_count;
    bool is_chosen;

    boost::shared_ptr< BaseAttributeHandle > value_handle;
    void user_changed_choice();
    void update_visibility();

public:
    ChoiceNode( boost::shared_ptr<Node> n, std::string name )
        : InnerNode(n, name), choice( new ChoiceWidget*() ), visible(true), choices_count(0), is_chosen(false) {}
    ~ChoiceNode();
    virtual void set_description( std::string d ) { description = d; }
    void initialization_finished();
    NodeHandle create_object( std::string name );
    NodeHandle create_group( std::string name );
    void add_attribute( simparm::BaseAttribute& a );
};

}
}

#endif
