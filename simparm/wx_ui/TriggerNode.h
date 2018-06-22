#ifndef SIMPARM_WX_UI_TRIGGERNODE_H
#define SIMPARM_WX_UI_TRIGGERNODE_H

#include "simparm/wx_ui/AttributeHandle.h"
#include "simparm/wx_ui/WindowNode.h"

namespace simparm {
namespace wx_ui {

class TriggerNode : public InnerNode {
    std::string description;
    boost::shared_ptr< AttributeHandle<unsigned long> > value;

public:
    TriggerNode( boost::shared_ptr<Node> n, std::string name ) : InnerNode(n, name) {}
    ~TriggerNode();
    virtual void set_description( std::string d ) { description = d; }
    void initialization_finished();
    void add_attribute( simparm::BaseAttribute& a ) {
        if ( a.get_name() == "value" ) value.reset( new AttributeHandle<unsigned long>(a, get_protocol()) );
    }
};

}
}

#endif
