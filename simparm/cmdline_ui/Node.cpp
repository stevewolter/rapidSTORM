#include "Node.h"
#include <sstream>
#include "ChoiceNode.h"
#include "EntryNode.h"
#include "ProgressNode.h"
#include "OptionTable.h"

namespace simparm {
namespace cmdline_ui {

Node::~Node() { 
    if ( parent ) parent->remove_child(*this); 
    while ( ! nodes.empty() ) {
        nodes.back()->parent = NULL;
        nodes.pop_back();
    }
}

void Node::add_child( Node& o ) {
    nodes.push_back( &o );
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
    o.parent = NULL;
}

simparm::NodeHandle Node::create_object( std::string name ) {
    return adorn_node( new Node(name) );
}

simparm::NodeHandle Node::create_group( std::string name ) { return adorn_node( new Node(name) ); }
simparm::NodeHandle Node::create_tab_group( std::string name ) { return adorn_node( new Node(name) ); }

simparm::NodeHandle Node::create_entry( std::string name, std::string ) {
    return adorn_node( new EntryNode( name, OptionTable::Value ) );
}

simparm::NodeHandle Node::create_choice( std::string name ) {
    return adorn_node( new ChoiceNode( name ) );
}

simparm::NodeHandle Node::adorn_node( Node* n ) {
    std::auto_ptr<Node> rv( n );
    add_child( *rv );
    return simparm::NodeHandle(rv.release());
}

simparm::NodeHandle Node::create_file_entry( std::string name ) {
    return adorn_node( new EntryNode( name, OptionTable::Value ) );
}

simparm::NodeHandle Node::create_progress_bar( std::string name ) {
    return adorn_node( new ProgressNode( name ) );
}

simparm::NodeHandle Node::create_trigger( std::string name ) {
    return adorn_node( new EntryNode( name, OptionTable::Trigger ) );
}

void Node::add_attribute( simparm::BaseAttribute& a ) {
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
