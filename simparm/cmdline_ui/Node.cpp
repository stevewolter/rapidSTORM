#include "simparm/cmdline_ui/Node.h"
#include <sstream>
#include "simparm/cmdline_ui/ChoiceNode.h"
#include "simparm/cmdline_ui/EntryNode.h"
#include "simparm/cmdline_ui/ProgressNode.h"
#include "simparm/cmdline_ui/OptionTable.h"

namespace simparm {
namespace cmdline_ui {

Node::~Node() { 
    assert( nodes.empty() );
    if ( parent ) parent->remove_child(*this); 
}

void Node::add_child( Node& o ) {
    nodes.push_back( &o );
    o.parent = shared_from_this();
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
}

simparm::NodeHandle Node::create_object( std::string name ) {
    return adorn_node( new Node(name) );
}

simparm::NodeHandle Node::create_group( std::string name ) { return adorn_node( new Node(name) ); }
simparm::NodeHandle Node::create_tab_group( std::string name ) { return adorn_node( new Node(name) ); }

simparm::NodeHandle Node::create_textfield( std::string name, std::string ) {
    return adorn_node( new EntryNode( name, OptionTable::Value ) );
}

simparm::NodeHandle Node::create_checkbox( std::string name ) {
    return adorn_node( new EntryNode( name, OptionTable::Boolean ) );
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

NodeHandle Node::create_tree_root() { return shared_from_this(); }
NodeHandle Node::create_tree_object( std::string name ) {
    return create_object( name );
}

std::auto_ptr<dStorm::display::WindowHandle> Node::get_image_window( 
    const dStorm::display::WindowProperties& wp, dStorm::display::DataSource& ds )
{
    return parent->get_image_window( wp, ds );
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
