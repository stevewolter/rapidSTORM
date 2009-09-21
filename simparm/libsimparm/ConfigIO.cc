#include "IO.hh"
#include "Set.hh"

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
    erase(name);
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
    ((IO*)configIO)->processInput();
    return NULL;
}

IO::~IO() {
    if (subthread_if_any) {
        pthread_join( *(pthread_t*)subthread_if_any, NULL );
        delete (pthread_t*)subthread_if_any;
    }

    removeAllChildren();
    
    if ( remoteAttached() )
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

    if ( cmd == "set" || cmd == "forSet" || cmd == "in" ) {
        string setn;
        in >> setn;
        try {
            (*this)[setn].processCommand(in);
        } catch (const std::invalid_argument& e) {
            cerr << e.what() << endl;
        }
    } else if (cmd == "attach") {
        remoteAttached = true;
        setActivity( true );
        std::for_each( begin(), end(), 
                        bind1st(mem_fun(&IO::define),this) );
        print("attach");
    } else if (cmd == "detach") {
        if (remoteAttached) {
            remoteAttached = false;
            setActivity( false );
            print("detach");
        }
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
    }
}

void IO::print(const std::string& what) {
    if (!out) return;
    pthread_mutex_lock( (pthread_mutex_t*)mutex );
    (*out) << what << endl;
    out->flush();
    pthread_mutex_unlock( (pthread_mutex_t*)mutex );
}

}
