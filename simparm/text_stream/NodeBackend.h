#ifndef SIMPARM_TEXT_STREAM_BACKENDNODE_H
#define SIMPARM_TEXT_STREAM_BACKENDNODE_H

#include "simparm/text_stream/FrontendNode.h"
#include "simparm/Message.h"
#include <boost/thread/recursive_mutex.hpp>
#include "display/Manager.h"

namespace simparm {
namespace text_stream {

class BackendNode {
  public:
    virtual ~BackendNode();
    void print( const std::string& );
    Message::Response send( const Message& );
    void detach_frontend() { detach_frontend_(); }

    static std::auto_ptr<BackendNode> make_child( std::string name, std::string type, FrontendNode& frontend, boost::shared_ptr<BackendNode> parent );

    virtual const std::string& get_name() const = 0;
    virtual std::ostream* get_print_stream() = 0;
    virtual Message::Response send_( const Message& ) = 0;
    virtual void process_child_command_( const std::string& child, std::istream& rest ) = 0;
    virtual void process_command_( const std::string& command, std::istream& rest );
    virtual boost::recursive_mutex* get_mutex() = 0;
    virtual void add_child( BackendNode& ) = 0;
    virtual void remove_child( BackendNode& ) = 0;
    virtual void declare( std::ostream& ) = 0;
    virtual std::auto_ptr<dStorm::display::WindowHandle> get_image_window( 
        const dStorm::display::WindowProperties&, dStorm::display::DataSource& ) = 0;
    virtual void detach_frontend_() = 0;

    void processCommand_( std::istream& );

};

}
}

#endif
