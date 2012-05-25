#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "RootNode.h"
#include "../Message.h"

#include <algorithm>
#include <sstream>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

using namespace std;

namespace simparm {
namespace text_stream {

RootNode::RootNode() 
: Node("IO","IO"),
  out(NULL), mutex(new pthread_mutex_t),
  is_attached(false), should_quit(false)
{
    pthread_mutex_init((pthread_mutex_t*)mutex, NULL);
}

RootNode::~RootNode() {
    if ( is_attached && out != NULL )
        (*out) << "quit" << std::endl;

    pthread_mutex_destroy((pthread_mutex_t*)mutex);
    delete (pthread_mutex_t*)mutex;
}

void RootNode::processCommand(const std::string& cmd, istream& in) {
    if (cmd == "attach") {
        is_attached = true;
        print("desc set " PACKAGE_STRING);
        print("viewable set true");
        print("userLevel set 10");
        print("showTabbed set true");

        declare_children();

        print("attach");
    } else if (cmd == "detach") {
        if (is_attached) {
            is_attached = false;
            hide();
            print_unconditionally("detach");
        }
    } else if (cmd == "quit") {
        should_quit = true;
    } else if (cmd == "cmd") {
        int number;
        in >> number;
        processCommand(in);
        std::stringstream response;
        response << "ack " << number;
        print_unconditionally( response.str() );
    } else if (cmd == "nop") {
        /* Do nothing. */
    } else if (cmd == "echo") {
        std::string s;
        std::getline(in, s);
        print_unconditionally(s);
    } else {
        Node::processCommand( cmd, in );
    }
}

void RootNode::print_unconditionally( const std::string& what ) {
    pthread_mutex_lock( (pthread_mutex_t*)mutex );
    (*out) << what << endl;
    out->flush();
    pthread_mutex_unlock( (pthread_mutex_t*)mutex );
}

bool RootNode::print(const std::string& what) {
    if (!out || !is_attached) return false;
    print_unconditionally(what);
    return true;
}

bool RootNode::print_on_top_level(const std::string& what) {
    return print( what );
}

void RootNode::set_output_stream( ostream *out ) { this->out = out; }

Message::Response RootNode::send( Message& m ) const {
    if ( out != NULL && is_attached ) {
        pthread_mutex_lock( (pthread_mutex_t*)mutex );
        (*out) << "declare simparmMessage\n"
              << "title set " << m.title << "\n"
              << "message set " << m.message << "\n"
              << "severity set " << m.severity << "\n"
              << "options set " << m.options << "\n"
              << "helpID set " << m.helpID << "\n"
              << "end\n";
        out->flush();
        pthread_mutex_unlock( (pthread_mutex_t*)mutex );
        return Message::OKYes;
    } else {
        std::cerr << m;
        return Message::OKYes;
    }
}

}
}
