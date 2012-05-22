#include "IO.hh"
#include "Set.hh"
#include "Message.hh"

#include <algorithm>
#include <fstream>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "stl_helpers.hh"

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

    Node::print_.connect( boost::bind( &IO::print, this, _1 ) );
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
            print("detach");
        }
#if 0
    } else if (cmd == "help") {
        if ( out )
            for ( iterator i = begin(); i != end(); i++ )
                i->printHelp( *out );
#endif
    } else if (cmd == "quit") {
        should_quit = true;
    } else if (cmd == "cmd") {
        int number;
        in >> number;
        processCommand(in);
        std::stringstream response;
        response << "ack " << number;
        print( response.str() );
    } else if (cmd == "nop") {
        /* Do nothing. */
    } else if (cmd == "echo") {
        std::string s;
        std::getline(in, s);
        print(s);
    } else {
        Node::processCommand( cmd, in );
    }
}

bool IO::print(const std::string& what) {
    if (!out || !remoteAttached()) return false;
    pthread_mutex_lock( (pthread_mutex_t*)mutex );
    (*out) << what << endl;
    out->flush();
    pthread_mutex_unlock( (pthread_mutex_t*)mutex );
    return true;
}

bool IO::print_on_top_level(const std::string& what) {
    return print( what );
}

void IO::set_input_stream( istream *in ) { this->in = in; }
void IO::set_output_stream( ostream *out ) { this->out = out; }

void IO::send( Message& m ) {
    if ( in != NULL && remoteAttached() ) {
#if 0
        /* Write the help_file */
        if ( m.help_file() == "" && has_child_named("help_file") ) {
            const Node& hf = (*this)["help_file"];
            const Attribute<std::string>* shf = dynamic_cast< const Attribute<std::string>* >( &hf );
            if ( shf != NULL ) m.help_file = *shf;
        }
        std::string definition = m.define();
        print( m.define() );
        print( m.undefine() );
#endif
    } else {
        std::cerr << m;
        m.set_response( Message::OKYes );
    }
}

}
