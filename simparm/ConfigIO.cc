#include "IO.hh"
#include "Set.hh"
#include "Message.hh"

#include <algorithm>
#include <fstream>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

using namespace std;

namespace simparm {

IO::IO(istream* in, ostream* out) 
: Node("IO","IO"),
  in(in), out(out), subthread_if_any(NULL), mutex(new pthread_mutex_t),
  should_quit(false),
  remoteAttached("remote_attached", false),
  showTabbed("showTabbed", false)
{
    //push_back( remoteAttached );
    add_attribute(showTabbed);

    pthread_mutex_init((pthread_mutex_t*)mutex, NULL);
}

#if 0
void IO::forkInput() {
    subthread_if_any = new pthread_t;
    int result = pthread_create( (pthread_t*)subthread_if_any,
        NULL, processInputCallback, this );
    if (result != 0) {
        delete (pthread_t*)subthread_if_any;
        subthread_if_any = NULL;
        throw std::runtime_error("Could not fork subthread: " +
            string(strerror(errno)));
    }
}

void *IO::processInputCallback(void *configIO) {
    try {
        ((IO*)configIO)->processInput();
    } catch (const std::exception& e) {
        std::cerr << "Uncaught error while processing simparm commands: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Caught unknown exception during input file reading" 
                  << std::endl;
    }
    return NULL;
}
#endif

IO::~IO() {
    if (subthread_if_any) {
        pthread_join( *(pthread_t*)subthread_if_any, NULL );
        delete (pthread_t*)subthread_if_any;
    }

    if ( remoteAttached() && out != NULL )
        (*out) << "quit" << std::endl;

    pthread_mutex_destroy((pthread_mutex_t*)mutex);
    delete (pthread_mutex_t*)mutex;
}

void IO::processInput() {
    while ( !should_quit && in && *in ) {
        processCommand(*in);
    }
}

void IO::processCommand(const std::string& cmd, istream& in) {
    if (cmd == "attach") {
        remoteAttached = true;
        if ( out ) show_attributes(*out);
        show_children();
        print("attach");
    } else if (cmd == "detach") {
        if (remoteAttached) {
            remoteAttached = false;
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

void IO::print_unconditionally( const std::string& what ) {
    pthread_mutex_lock( (pthread_mutex_t*)mutex );
    (*out) << what << endl;
    out->flush();
    pthread_mutex_unlock( (pthread_mutex_t*)mutex );
}

bool IO::print(const std::string& what) {
    if (!out || !remoteAttached()) return false;
    print_unconditionally(what);
    return true;
}

bool IO::print_on_top_level(const std::string& what) {
    return print( what );
}

void IO::set_input_stream( istream *in ) { this->in = in; }
void IO::set_output_stream( ostream *out ) { this->out = out; }

Message::Response IO::send( Message& m ) {
    if ( out != NULL && remoteAttached() ) {
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
