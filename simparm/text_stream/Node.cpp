#include "simparm/text_stream/Node.h"
#include "simparm/text_stream/TabNode.h"
#include "simparm/text_stream/EntryNode.h"
#include "simparm/text_stream/NoOpNode.h"
#include <sstream>

namespace simparm {
namespace text_stream {

Node::Node( std::string name, std::string type ) 
: name(name), type(type), 
  desc("desc", ""),
  viewable("viewable", true),
  userLevel("userLevel", Beginner)
{
    add_attribute( desc );
    add_attribute( viewable );
    add_attribute( userLevel );
}

Node::~Node() {
    backend_node->detach_frontend();
}

template <typename Type>
struct equal_address {
    Type* a;
    equal_address( Type* a ) : a(a) {}
    typedef bool result_type;
    bool operator()( Type& o ) const { return &o == a; }
    bool operator()( Type* o ) const { return o == a; }
};

simparm::NodeHandle Node::create_object( std::string name ) {
    return adorn_node( new Node( name, "Object" ) );
}

simparm::NodeHandle Node::create_group( std::string name ) {
    return adorn_node( new TabNode( name, false ) );
}

simparm::NodeHandle Node::create_tab_group( std::string name ) {
    return adorn_node( new TabNode( name, true ) );
}

simparm::NodeHandle Node::create_textfield( std::string name, std::string type ) {
    return adorn_node( new EntryNode( name, type + "Entry" ) );
}

simparm::NodeHandle Node::create_checkbox( std::string name ) {
    return adorn_node( new EntryNode( name, "BoolEntry" ) );
}

simparm::NodeHandle Node::create_choice( std::string name ) {
    return adorn_node( new EntryNode( name, "ChoiceEntry" ) );
}

simparm::NodeHandle Node::adorn_node( Node* n ) {
    std::auto_ptr<Node> rv( n );
    rv->parent = this;
    return simparm::NodeHandle(rv.release());
}

simparm::NodeHandle Node::create_file_entry( std::string name ) {
    return adorn_node( new EntryNode( name, "FileEntry" ) );
}

simparm::NodeHandle Node::create_progress_bar( std::string name ) {
    return adorn_node( new EntryNode( name, "ProgressEntry" ) );
}

simparm::NodeHandle Node::create_trigger( std::string name ) {
    return adorn_node( new EntryNode( name, "TriggerEntry" ) );
}

simparm::NodeHandle Node::create_tree_root() { return NodeHandle( new NoOpNode( backend_node ) ); }
simparm::NodeHandle Node::create_tree_object( std::string name ) {
    return adorn_node( new Node( name, "Object" ) );
}

std::string Node::attribute_value_specification( const BaseAttribute& a )
{
    boost::optional<std::string> v = a.get_value();
    if ( v )
        return "set " + *v;
    else
        return "unset";
}

void Node::add_attribute( simparm::BaseAttribute& a ) {
    attributes.add( a );
    connections.push_back( a.notify_on_value_change( boost::bind( &Node::print_attribute_value, this, boost::cref(a) ) ) );
}

Message::Response Node::send( Message& m ) const {
    return backend_node->send( m );
}

void Node::print_attribute_value( const simparm::BaseAttribute& a ) {
    if ( backend_node.get() )
        backend_node->print( "in " + a.get_name() + " " + attribute_value_specification(a) );
}

void Node::declare_attribute( const BaseAttribute* a, std::ostream& d ) {
    if ( a )
        d << a->get_name() << " " << attribute_value_specification(*a) << "\n";
}
void Node::declare_( std::ostream& declaration ) {
    attributes.for_each( boost::bind( &Node::declare_attribute, this, _1, boost::ref(declaration) ) );
}

std::auto_ptr<dStorm::display::WindowHandle> Node::get_image_window( 
    const dStorm::display::WindowProperties& wp, dStorm::display::DataSource& ds )
{
    return backend_node->get_image_window( wp, ds );
}

void Node::initialization_finished() {
    assert( parent );
    assert( parent->backend_node.get() );
    backend_node = BackendNode::make_child( name, type, *this, parent->backend_node );
}

/** TODO: Method is deprecated and should be removed on successful migration. */
bool Node::isActive() const { return true; }

void Node::process_attribute_command_( std::string name, std::istream& rest ) {
    BaseAttribute* a = attributes.look_up( name );
    if ( ! a ) 
        throw std::runtime_error("Unknown node '" + name + "'");

    std::string command;
    rest >> command;
    if ( command == "query" ) {
        std::stringstream s;
        s << "in " << a->get_name() << " ";
        declare_attribute( a, s );
        backend_node->print( s.str() );
    } else if ( command == "set" ) {
        std::string line;
        std::getline( rest, line );
        bool successful = a->set_value( line );
        if ( ! successful )
            print_attribute_value( *a );
    } else if ( command == "unset" ) {
        a->unset_value();
    } else
        throw std::runtime_error("Unknown attribute command: " + command );
}

}
}
