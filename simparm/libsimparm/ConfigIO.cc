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
: Object("IO", "I/O processor"),
  in(in), out(out), subthread_if_any(NULL), mutex(new pthread_mutex_t),
  detached(false),
  should_quit(false),
  remoteAttached("remote_attached", false),
  showTabbed("showTabbed", false)
{
    //push_back( remoteAttached );
    push_back(showTabbed);

    pthread_mutex_init((pthread_mutex_t*)mutex, NULL);
}

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

IO::~IO() {
    if (subthread_if_any) {
        pthread_join( *(pthread_t*)subthread_if_any, NULL );
        delete (pthread_t*)subthread_if_any;
    }

    clearChildren();
    
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

void IO::processCommand(istream& in) {
    string cmd;
    in >> cmd;

    processCommand( cmd, in );
}

void IO::processCommand(const std::string& cmd, istream& in) {
    if ( cmd == "set" || cmd == "forSet" || cmd == "in" ) {
        string setn;
        in >> setn;
        (*this)[setn].processCommand(in);
    } else if (cmd == "attach") {
        remoteAttached = true;
        setActivity( true );
        for ( iterator i = begin(); i != end(); i++ )
            define( *i );
        print("attach");
    } else if (cmd == "detach") {
        if (remoteAttached) {
            remoteAttached = false;
            setActivity( false );
            print("detach");
        }
    } else if (cmd == "help") {
        if ( out )
            for ( iterator i = begin(); i != end(); i++ )
                i->printHelp( *out );
    } else if (cmd == "quit") {
        should_quit = true;
#if 0
        if (remoteAttached) {
            remoteAttached = false;
            setActivity( false );
            std::for_each( begin(), end(), 
                            bind1st(mem_fun(&IO::undefine),this) );
        }
        this->in = NULL;
#endif
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
    }
}

void IO::print(const std::string& what) {
    if (!out) return;
    pthread_mutex_lock( (pthread_mutex_t*)mutex );
    (*out) << what << endl;
    out->flush();
    pthread_mutex_unlock( (pthread_mutex_t*)mutex );
}

void IO::set_input_stream( istream *in ) { this->in = in; }
void IO::set_output_stream( ostream *out ) { this->out = out; }

void IO::send( Message& m ) {
    if ( in != NULL && remoteAttached() ) {
        /* Write the help_file */
        if ( m.help_file() == "" && has_child_named("help_file") ) {
            const Node& hf = (*this)["help_file"];
            const Attribute<std::string>* shf = dynamic_cast< const Attribute<std::string>* >( &hf );
            if ( shf != NULL ) m.help_file = *shf;
        }
        std::string definition = m.define();
        print( m.define() );
        print( m.undefine() );
    } else {
        std::cerr << m;
        m.set_response( Message::OKYes );
    }
}

}
