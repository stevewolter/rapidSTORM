#include "Node.h"
#include "TabNode.h"
#include "EntryNode.h"
#include <sstream>

namespace simparm {
namespace text_stream {

Node::Node( std::string name, std::string type ) 
: name(name), type(type), 
  desc("desc", ""),
  viewable("viewable", true),
  userLevel("userLevel", Beginner),
  parent(NULL), declared(false)
{
    add_attribute( desc );
    add_attribute( viewable );
    add_attribute( userLevel );
}

bool Node::print( const std::string& s ) { 
    if ( declared && parent )
        return parent->print("in " + name + " " + s); 
    else
        return false;
}

bool Node::print_on_top_level( const std::string& s ) { 
    if ( declared && parent )
        return parent->print_on_top_level(s); 
    else
        return false;
}

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

    print("remove " + o.name);
}

simparm::NodeHandle Node::create_object( std::string name ) {
    return adorn_node( new Node( name, "Object" ) );
}

simparm::NodeHandle Node::create_group( std::string name ) {
    return adorn_node( new TabNode( name, false ) );
}

simparm::NodeHandle Node::create_tab_group( std::string name ) {
    return adorn_node( new TabNode( name, true ) );
}

simparm::NodeHandle Node::create_entry( std::string name, std::string type ) {
    return adorn_node( new EntryNode( name, type + "Entry" ) );
}

simparm::NodeHandle Node::create_choice( std::string name ) {
    return adorn_node( new EntryNode( name, "ChoiceEntry" ) );
}

simparm::NodeHandle Node::adorn_node( Node* n ) {
    std::auto_ptr<Node> rv( n );
    add_child( *rv );
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

std::string Node::attribute_value_specification( const BaseAttribute& a )
{
    boost::optional<std::string> v = a.get_value();
    if ( v )
        return "set " + *v;
    else
        return "unset";
}

void Node::add_attribute( simparm::BaseAttribute& a ) {
    attributes.push_back( &a );
    attribute_lookup.insert( std::make_pair( a.get_name(), &a ) );
    connections.push_back( a.notify_on_value_change( boost::bind( &Node::print_attribute_value, this, boost::cref(a) ) ) );
}

Message::Response Node::send( Message& m ) const {
    if ( parent ) return parent->send( m ); else return Message::OKYes;
}

void Node::print_attribute_value( const simparm::BaseAttribute& a ) {
    print( "in " + a.get_name() + " " + attribute_value_specification(a) );
}

void Node::declare_children() {
    std::for_each( nodes.begin(), nodes.end(),
        boost::bind( &Node::initialization_finished, _1 ) );
}

void Node::show_attributes( std::ostream& declaration ) {
    for ( std::vector< BaseAttribute* >::const_iterator i = attributes.begin(); i != attributes.end(); ++i )
        declaration << (*i)->get_name() << " " << attribute_value_specification(**i) << "\n";
}

void Node::declare( std::ostream& o ) {
    if ( ! declared ) {
        declared = true;
        o << "declare " << type << "\n";
        o << "name " << name << "\n";
        show_attributes( o );
        std::for_each( nodes.begin(), nodes.end(),
            boost::bind( &Node::declare, _1, boost::ref(o) ) );
        o << "end\n";
    }
}

void Node::undeclare() {
    declared = false;
    std::for_each( nodes.begin(), nodes.end(),
        boost::bind( &Node::undeclare, _1 ) );
}

void Node::initialization_finished() {
    if ( ! parent ) return;
    std::stringstream declaration;
    declare( declaration );
    std::string d = declaration.str();
    /* Delete terminal newline that will be re-attached upon printing the command */
    if ( d != "" ) {
        d.erase( d.length() - 1 );
        bool did_print = parent->print( d );
        if ( ! did_print )
            undeclare();
    }
}

void Node::hide() {
    if ( declared ) {
        print( "remove " + name );
        declared = false;
        undeclare();
    }
}

/** TODO: Method is deprecated and should be removed on successful migration. */
bool Node::isActive() const {
    return declared;
}

void Node::processCommand( std::istream& is ) {
    std::string command;
    is >> command;
    if ( is )
        processCommand( command, is );
}

void Node::process_attribute( BaseAttribute& a, std::istream& rest ) {
    std::string command;
    rest >> command;
    if ( command == "query" )
        print( "in " + a.get_name() + " " + a.get_name() + " " + attribute_value_specification(a) );
    else if ( command == "set" ) {
        std::string line;
        std::getline( rest, line );
        a.set_value( line );
    } else if ( command == "unset" ) {
        a.unset_value();
    } else
        throw std::runtime_error("Unknown attribute command: " + command );
}

void Node::processCommand( const std::string& cmd, std::istream& rest ) {
    if ( cmd == "forSet" || cmd == "in" || cmd == "set" ) {
        std::string name;
        rest >> name;
        std::map< std::string, Node* >::const_iterator i = node_lookup.find(name);
        if ( i != node_lookup.end() )
            i->second->processCommand( rest );
        else {
            std::map< std::string, BaseAttribute* >::const_iterator j = attribute_lookup.find(name);
            if ( j != attribute_lookup.end() ) {
                process_attribute( *j->second, rest );
            } else
                throw std::runtime_error("Unknown node '" + name + "'");
        }
    } else if ( cmd == "value" ) {
        process_attribute( *attribute_lookup[cmd], rest );
    } else {
        std::string discard;
        std::getline( rest, discard );
        throw std::runtime_error("Unrecognized command " + cmd);
    }
}

}
}
