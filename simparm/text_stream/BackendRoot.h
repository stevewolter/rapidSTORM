#ifndef SIMPARM_BACKEND_ROOT_H
#define SIMPARM_BACKEND_ROOT_H

#include "simparm/text_stream/NodeBackend.h"
#include "simparm/text_stream/ChildrenList.h"
#include "simparm/text_stream/image_window/MainThread.h"

namespace simparm {
namespace text_stream {

class BackendRoot : public BackendNode {
    virtual const std::string& get_name() const;
    virtual std::ostream* get_print_stream();
    virtual Message::Response send_( const Message& );
    virtual void process_child_command_( const std::string& child, std::istream& rest );
    virtual void add_child( BackendNode& );
    virtual void remove_child( BackendNode& );
    virtual void declare( std::ostream& );
    virtual std::auto_ptr<dStorm::display::WindowHandle> get_image_window( 
        const dStorm::display::WindowProperties&, dStorm::display::DataSource& );
    void detach_frontend_() {}

    bool attached;
    bool should_quit;
    std::ostream* out;
    Mutex mutex;

    image_window::MainThread display_manager;
    NodeHandle wx_ui_handler;

    ChildrenList< BackendNode > children;
protected:
    virtual void process_command_( const std::string& command, std::istream& rest );
    virtual Mutex* get_mutex();
public:
    BackendRoot( std::ostream*, bool wxWidgets );
    ~BackendRoot();
    void processCommand( std::istream& );
    void attach_ui( simparm::NodeHandle h ) { display_manager.attach_ui(h); }
    bool received_quit_command() const;
};

}
}

#endif
