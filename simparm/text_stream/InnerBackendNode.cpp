#include "InnerBackendNode.h"

namespace simparm {
namespace text_stream {

InnerBackendNode::InnerBackendNode( std::string name, std::string type, FrontendNode& frontend, boost::shared_ptr<BackendNode> parent )
: name(name), type(type), parent(parent), declared(false), frontend( frontend ), tree_mutex( parent->get_mutex() )
{
    parent->add_child( *this );
}

void InnerBackendNode::add_child( BackendNode& t ) { 
    assert( &t );
    children.add( t ); 
    std::ostream* o = get_print_stream();
    if (o) t.declare( *o );
}

void InnerBackendNode::remove_child( BackendNode& t ) { 
    assert( children.look_up( t.get_name() ) == &t );
    children.remove(t); 
    std::ostream* o = get_print_stream();
    if ( o ) *o << "remove " << t.get_name() << std::endl;
    assert( children.look_up( t.get_name() ) == NULL );
}

InnerBackendNode::~InnerBackendNode() {
    boost::lock_guard<Mutex> m( *get_mutex() );
    parent->remove_child( *this );
} 

std::ostream* InnerBackendNode::get_print_stream() {
    if ( !declared ) return NULL;
    std::ostream* o = parent->get_print_stream();
    if ( o ) *o << "in " << name << " ";
    return o;
}

void InnerBackendNode::process_child_command_( const std::string& child, std::istream& rest ) {
    BackendNode* my_child = children.look_up( child );
    if ( my_child ) {
        my_child->processCommand_( rest );
    } else {
        frontend.process_attribute_command( child, rest );
    }
}

void InnerBackendNode::declare( std::ostream& o ) { 
    if ( ! declared ) {
        o << "declare " << type << "\n";
        o << "name " << name << "\n";
        frontend.declare( o );
        children.for_each( boost::bind( &BackendNode::declare, _1, boost::ref(o) ) );
        o << "end" << std::endl;
        declared = true;
    }
}

}
}
