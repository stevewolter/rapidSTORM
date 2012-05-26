#ifndef SIMPARM_TEXT_STREAM_BACKENDNODE_H
#define SIMPARM_TEXT_STREAM_BACKENDNODE_H

#include "FrontendNode.h"
#include "../Message.h"
#include <boost/thread/recursive_mutex.hpp>

namespace simparm {
namespace text_stream {

struct BackendNode {
    typedef boost::recursive_mutex Mutex;

    virtual const std::string& get_name() const = 0;
    virtual std::ostream* get_print_stream() = 0;
    virtual Message::Response send_( const Message& ) = 0;
    virtual void process_child_command_( const std::string& child, std::istream& rest ) = 0;
    virtual void process_command_( const std::string& command, std::istream& rest );
    virtual Mutex* get_mutex() = 0;
    virtual void add_child( BackendNode& ) = 0;
    virtual void remove_child( BackendNode& ) = 0;
    virtual void declare( std::ostream& ) = 0;

    void processCommand_( std::istream& );

public:
    virtual ~BackendNode();
    void print( const std::string& );
    Message::Response send( const Message& );

    static std::auto_ptr<BackendNode> make_child( std::string name, std::string type, FrontendNode& frontend, boost::shared_ptr<BackendNode> parent );
};

}
}

#endif
