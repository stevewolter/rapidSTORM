#ifndef SIMPARM_INNER_BACKEND_NODE_H
#define SIMPARM_INNER_BACKEND_NODE_H

#include "NodeBackend.h"
#include "ChildrenList.h"

#include <boost/bind/bind.hpp>

namespace simparm {
namespace text_stream {

struct InnerBackendNode : public BackendNode {
    std::string name;
    ChildrenList< BackendNode > children;
    boost::shared_ptr<BackendNode> parent;
    bool declared;

    FrontendNode& frontend;
    boost::recursive_mutex* tree_mutex;

    virtual std::ostream* get_print_stream() {
        if ( !declared ) return NULL;
        std::ostream* o = parent->get_print_stream();
        if ( o ) *o << "in " << name << " ";
        return o;
    }
    virtual Message::Response send_( const Message& m )
        { return parent->send_(m); }
    virtual void process_child_command_( const std::string& child, std::istream& rest ) {
        BackendNode* my_child = children.look_up( child );
        if ( my_child ) {
            my_child->processCommand_( rest );
        } else {
            frontend.process_attribute_command( child, rest );
        }
    }
    virtual const std::string& get_name() const {
        return name;
    }
    virtual Mutex* get_mutex() { return tree_mutex; }
    virtual void add_child( BackendNode& t );
    virtual void remove_child( BackendNode& t ); 
    virtual void declare( std::ostream& o ) { 
        if ( ! declared ) {
            o << "declare\n";
            frontend.declare( o );
            children.for_each( boost::bind( &BackendNode::declare, _1, boost::ref(o) ) );
            o << "end\n";
            declared = true;
        }
    }

public:
    InnerBackendNode( std::string name, FrontendNode& frontend, boost::shared_ptr<BackendNode> parent );
    ~InnerBackendNode(); 
};

}
}

#endif
