#include "Node.h"
#include <sstream>
#include "TriggerNode.h"

namespace simparm {
namespace cmdline_ui {

Node::~Node() { 
    if ( parent ) parent->remove_child(*this); 
    while ( ! nodes.empty() )
        remove_child( *nodes.back() );
}

void Node::add_child( Node& o ) {
    nodes.push_back( &o );
    node_lookup.insert( std::make_pair( o.name, &o ) );
    o.parent = this;
}

template <typename Type>
struct equal_address {
    Type* a;
    equal_address( Type* a ) : a(a) {}
    typedef bool result_type;
    bool operator()( Type& o ) const { return &o == a; }
    bool operator()( Type* o ) const { return o == a; }
};

void Node::remove_child( Node& o ) {
    nodes.erase( std::remove_if( nodes.begin(), nodes.end(), equal_address<Node>(&o) ) );
    node_lookup.erase( o.name );
    o.parent = NULL;
}

simparm::NodeHandle Node::create_object( std::string name ) {
    return create_node( name );
}

simparm::NodeHandle Node::create_set( std::string name ) {
    return create_node( name );
}

simparm::NodeHandle Node::create_entry( std::string name, std::string, std::string ) {
    return create_node( name );
}

simparm::NodeHandle Node::create_choice( std::string name, std::string ) {
    return create_node( name );
}

simparm::NodeHandle Node::create_node( std::string name ) {
    std::auto_ptr<Node> rv( new Node(name) );
    add_child( *rv );
    return simparm::NodeHandle(rv.release());
}

simparm::NodeHandle Node::create_file_entry( std::string name, std::string ) {
    return create_node( name );
}

simparm::NodeHandle Node::create_progress_bar( std::string name, std::string ) {
    return create_node( name );
}

simparm::NodeHandle Node::create_trigger( std::string name, std::string ) {
    std::auto_ptr<Node> rv( new TriggerNode(name) );
    add_child( *rv );
    return simparm::NodeHandle(rv.release());
}

void Node::add_attribute( simparm::BaseAttribute& a ) {
    attributes.push_back( &a );
    attribute_lookup.insert( std::make_pair( a.get_name(), &a ) );
}

Message::Response Node::send( Message& m ) const {
    if ( parent ) return parent->send( m ); else return Message::OKYes;
}

void Node::program_options( OptionTable& o ) {
    std::for_each( nodes.begin(), nodes.end(),
        boost::bind( &Node::program_options, _1, boost::ref(o) ) );
}

}
}
