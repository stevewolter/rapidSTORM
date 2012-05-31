#ifndef SIMPARM_WX_UI_TRIGGERNODE_H
#define SIMPARM_WX_UI_TRIGGERNODE_H

#include "WindowNode.h"
#include "AttributeHandle.h"

namespace simparm {
namespace wx_ui {

class TriggerNode : public Node {
    std::string description;
    boost::shared_ptr< AttributeHandle<unsigned long> > value;

public:
    TriggerNode( boost::shared_ptr<Node> n )
        : Node(n) {}
    ~TriggerNode();
    virtual void set_description( std::string d ) { description = d; }
    void initialization_finished();
    void add_attribute( simparm::BaseAttribute& a ) {
        if ( a.get_name() == "value" ) value.reset( new AttributeHandle<unsigned long>(a) );
    }
};

}
}

#endif
