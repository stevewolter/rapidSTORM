#include "Node.h"
#include <sstream>

namespace simparm {
namespace text_stream {

bool Node::print( const std::string& s ) { 
    if ( declared )
        return print_("in " + name + " " + s); 
    else
        return false;
}

bool Node::print_on_top_level( const std::string& s ) { 
    if ( declared )
        return print_top_level_(s); 
    else
        return false;
}

void Node::listen_to( Node& o ) {
    o.print_.connect( boost::bind( &Node::print, this, _1 ) );
    o.print_top_level_.connect( boost::bind( &Node::print_on_top_level, this, _1 ) );
}

std::auto_ptr<simparm::Node> Node::create_object( std::string name ) {
    return create_node( name, "Object" );
}

std::auto_ptr<simparm::Node> Node::create_set( std::string name ) {
    return create_node( name, "Set" );
}

std::auto_ptr<simparm::Node> Node::create_entry( std::string name, std::string desc, std::string type ) {
    return create_node( name, type + "Entry" );
}

std::auto_ptr<simparm::Node> Node::create_choice( std::string name, std::string desc ) {
    return create_node( name, "ChoiceEntry" );
}

std::auto_ptr<simparm::Node> Node::create_node( std::string name, std::string type ) {
    std::auto_ptr<Node> rv( new Node(name,type) );
    listen_to( *rv );
    nodes.push_back( rv.get() );
    node_lookup.insert( std::make_pair( name, rv.get() ) );
    return std::auto_ptr<simparm::Node>(rv.release());
}

std::auto_ptr<simparm::Node> Node::create_file_entry( std::string name, std::string ) {
    return create_node( name, "FileEntry" );
}

std::auto_ptr<simparm::Node> Node::create_progress_bar( std::string name, std::string ) {
    return create_node( name, "ProgressEntry" );
}

std::auto_ptr<simparm::Node> Node::create_trigger( std::string name, std::string ) {
    return create_node( name, "TriggerEntry" );
}

void Node::add_attribute( simparm::BaseAttribute& a ) {
    attributes.push_back( &a );
    attribute_lookup.insert( std::make_pair( a.get_name(), &a ) );
    a.notify_on_value_change( boost::bind( &Node::print_attribute_value, this, boost::cref(a) ) );
}

void Node::send( Message& ) {
}

void Node::print_attribute_value( const simparm::BaseAttribute& a ) {
    print( "in " + a.get_name() + " " + a.get_value() );
}

void Node::show_attributes( std::ostream& declaration ) {
    for ( std::vector< BaseAttribute* >::const_iterator i = attributes.begin(); i != attributes.end(); ++i )
        declaration << (*i)->get_name() << " " << (*i)->get_value() << "\n";
}

void Node::show_children() {
    for ( std::vector< Node* >::const_iterator i = nodes.begin(); i != nodes.end(); ++i ) {
        (*i)->show();
    }
}

void Node::declare( std::ostream& o ) {
    if ( ! declared ) {
        declared = true;
        o << "declare " << type << "\n";
        o << "name " << name << "\n";
        show_attributes( o );
        for ( std::vector< Node* >::const_iterator i = nodes.begin(); i != nodes.end(); ++i ) {
            (*i)->declare(o);
        }
        o << "end\n";
    }
}

void Node::undeclare() {
    declared = false;
    for ( std::vector< Node* >::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
        (*i)->undeclare();
}

void Node::show() {
    std::stringstream declaration;
    declare( declaration );
    bool did_print = print_( declaration.str() );
    if ( ! did_print )
        undeclare();
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

#if 0
    void processCommand(std::istream& from) {
        std::string command;
        from >> command;
        
        Type value;
        if ( AttributeCommandInterpreter<Type>::from_stream(command, from, value) ) {
            valueChange( value, false );
        } else if ( command == "query" )
            print( define() );
        else
            throw std::runtime_error("Unrecognized command " + command);
    }
#endif

void Node::process_attribute( BaseAttribute& a, std::istream& rest ) {
    std::string command;
    rest >> command;
    if ( command == "query" )
        print( "in " + a.get_name() + " " + a.get_name() + " " + a.get_value() );
    else 
        a.set_value( command, rest );
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
