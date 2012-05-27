#include "NodeBackend.h"
#include "InnerBackendNode.h"

namespace simparm {
namespace text_stream {

BackendNode::~BackendNode() {}

void BackendNode::print( const std::string& s ) {
    boost::lock_guard< Mutex > m( *get_mutex() );
    std::ostream* o = get_print_stream();
    if ( o ) *o << s << std::endl;
}

void BackendNode::process_command_( const std::string& cmd, std::istream& rest ) {
    if ( cmd == "forSet" || cmd == "in" || cmd == "set" ) {
        std::string name;
        rest >> name;
        process_child_command_( name, rest );
    } else if ( cmd == "value" ) {
        process_child_command_( cmd, rest );
    } else {
        std::string discard;
        std::getline( rest, discard );
        throw std::runtime_error("Unrecognized command " + cmd);
    }
}

void BackendNode::processCommand_( std::istream& i ) {
    std::string command;
    i >> command;
    if ( i )
        process_command_(command,i);
    else 
        throw std::runtime_error("Unexpected end of config stream");
}

Message::Response BackendNode::send( const Message& m ) {
    boost::lock_guard< Mutex > lock( *get_mutex() );
    return send_( m );
}

std::auto_ptr<BackendNode> BackendNode::make_child( std::string name, std::string type, FrontendNode& frontend, boost::shared_ptr<BackendNode> parent ) {
    boost::lock_guard< Mutex > m( *parent->get_mutex() );
    return std::auto_ptr<BackendNode>( new InnerBackendNode( name, type, frontend, parent ) );
}

}
}
