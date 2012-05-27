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
    if ( o ) *o << "remove " << t.get_name() << "\n";
    assert( children.look_up( t.get_name() ) == NULL );
}

InnerBackendNode::~InnerBackendNode() {
    boost::lock_guard<Mutex> m( *get_mutex() );
    parent->remove_child( *this );
} 

}
}
