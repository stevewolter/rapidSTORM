#ifndef SIMPARM_TEXT_STREAM_NOOP_NODE_H
#define SIMPARM_TEXT_STREAM_NOOP_NODE_H

#include "simparm/text_stream/Node.h"

namespace simparm {
namespace text_stream {

class BackendNode;

struct NoOpNode 
: public Node
{
    virtual void set_visibility( bool ) {}
    virtual void set_user_level( UserLevel ) {}
    virtual void set_description( std::string ) {}
    virtual void set_help_id( std::string ) {}
    virtual void set_help( std::string ) {}
    virtual void set_editability( bool ) {}

public:
    NoOpNode( boost::shared_ptr<BackendNode> backend ) 
        : Node("Foo", "Foo") { set_backend_node(backend); }

    void add_attribute( simparm::BaseAttribute& ) {}
    void initialization_finished() {}
    void hide() {}
    bool isActive() const { return true; }
};

}
}

#endif
