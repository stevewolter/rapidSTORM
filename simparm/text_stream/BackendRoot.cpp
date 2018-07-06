#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "simparm/text_stream/BackendRoot.h"
#include <boost/bind/bind.hpp>
#include "simparm/wx_ui/no_main_window.h"
#include "simparm/dummy_ui/fwd.h"
#include "simparm/Node.h"

namespace simparm {
namespace text_stream {

BackendRoot::BackendRoot( std::ostream* o, bool wxWidgets )
: attached(false), should_quit(false), out(o),
  display_manager()
{
    if ( wxWidgets )    
        wx_ui_handler = wx_ui::no_main_window();
}

BackendRoot::~BackendRoot() {
    if ( attached && out )
        (*out) << "quit" << std::endl;
}

const std::string& BackendRoot::get_name() const { throw std::logic_error("Method was thought unneeded"); }
std::ostream* BackendRoot::get_print_stream() { return out; }
BackendRoot::Mutex* BackendRoot::get_mutex() { return &mutex; }
void BackendRoot::add_child( BackendNode& b ) { 
    children.add(b); 
    if ( attached && out )
        b.declare( *out );
}
void BackendRoot::remove_child( BackendNode& b ) { 
    children.remove(b); 
    if ( attached && out )
        *out << "remove " << b.get_name() << std::endl;
}
void BackendRoot::declare( std::ostream& ) { throw std::logic_error("Method was thought unneeded"); }

void BackendRoot::processCommand( std::istream& i ) {
    boost::lock_guard< Mutex > m( *get_mutex() );
    BackendNode::processCommand_( i );
}

void BackendRoot::process_command_(const std::string& cmd, std::istream& in) {
    if (cmd == "attach") {
        attached = true;
        if ( out ) {
            *out << "desc set " PACKAGE_STRING "\n";
            *out << "viewable set true\n";
            *out << "userLevel set 10\n";
            *out << "showTabbed set true\n";
            children.for_each( boost::bind( &BackendNode::declare, _1, boost::ref(*out) ) );
            *out << ("attach") << std::endl;
        }
    } else if (cmd == "detach") {
        if (attached) {
            attached = false;
            if (out) *out << "detach" << std::endl;
        }
    } else if (cmd == "quit") {
        should_quit = true;
        display_manager.attach_ui( dummy_ui::make_node() );
        if (out) *out << "quit" << std::endl;
    } else if (cmd == "cmd") {
        int number;
        in >> number;
        processCommand(in);
        if ( out ) *out  << "ack " << number << std::endl;
    } else if (cmd == "nop") {
        /* Do nothing. */
    } else if (cmd == "echo") {
        std::string s;
        std::getline(in, s);
        if ( out ) *out << s << std::endl;
    } else {
        BackendNode::process_command_( cmd, in );
    }
}

Message::Response BackendRoot::send_( const Message& m ) {
    if ( out != NULL && attached ) {
        (*out) << "declare simparmMessage\n"
              << "title set " << m.title << "\n"
              << "message set " << m.message << "\n"
              << "severity set " << m.severity << "\n"
              << "options set " << m.options << "\n"
              << "helpID set " << m.helpID << "\n"
              << "end" << std::endl;
        return Message::OKYes;
    } else {
        std::cerr << m;
        return Message::OKYes;
    }
}

bool BackendRoot::received_quit_command() const {
    return should_quit;
}

void BackendRoot::process_child_command_( const std::string& child, std::istream& rest ) {
    BackendNode* my_child = children.look_up( child );
    if ( my_child ) {
        my_child->processCommand_( rest );
    } else {
        throw std::runtime_error("Unknown node " + child);
    }
}

std::auto_ptr<dStorm::display::WindowHandle> BackendRoot::get_image_window( 
    const dStorm::display::WindowProperties& wp, dStorm::display::DataSource& ds ) 
{
    if ( wx_ui_handler )
        return wx_ui_handler->get_image_window( wp, ds );
    else
        return display_manager.register_data_source( wp, ds );
}

}
}
