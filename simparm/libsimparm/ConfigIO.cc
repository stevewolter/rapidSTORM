#include "ConfigIO.hh"
#include "ConfigSet.hh"

#include <algorithm>
#include <fstream>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "stl_helpers.hh"

using namespace std;

namespace simparm {

ConfigIO::ConfigIO(istream* in, ostream* out) throw() 
: ConfigObject("IO", "I/O processor"),
  in(in), out(out), subthread_if_any(NULL), mutex(new pthread_mutex_t),
  detached(false),
  remoteAttached("remote_attached", false)
{
    erase(name);
    erase(desc);
    //push_back( remoteAttached );

    pthread_mutex_init((pthread_mutex_t*)mutex, NULL);
}

void ConfigIO::forkInput() throw(std::runtime_error) {
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

void *ConfigIO::processInputCallback(void *configIO) throw() {
    ((ConfigIO*)configIO)->processInput();
    return NULL;
}

ConfigIO::~ConfigIO() throw() {
    if (subthread_if_any) {
        pthread_join( *(pthread_t*)subthread_if_any, NULL );
        delete (pthread_t*)subthread_if_any;
    }

    while ( ! children.empty() )
        erase( *children.front() );

    pthread_mutex_destroy((pthread_mutex_t*)mutex);
    delete (pthread_mutex_t*)mutex;
}

void ConfigIO::processInput() throw() {
    while ( in && *in ) {
        processCommand(*in);
    }
}

void ConfigIO::processCommand(istream& in) throw() {
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
        nstd::for_each( children, 
                        bind1st(mem_fun(&ConfigIO::define),this) );
        print("attach");
    } else if (cmd == "detach") {
        remoteAttached = false;
        setActivity( false );
        print("detach");
    } else if (cmd == "quit") {
        remoteAttached = false;
        setActivity( false );
        nstd::for_each( children, 
                        bind1st(mem_fun(&ConfigIO::undefine),this) );
        this->in = NULL;
    } 
    /* For debugging. */
    else if (cmd == "sleep") {
        sleep(1);
    }
}

void ConfigIO::print(const std::string& what) throw() {
    if (!out) return;
    pthread_mutex_lock( (pthread_mutex_t*)mutex );
    (*out) << what << endl;
    out->flush();
    pthread_mutex_unlock( (pthread_mutex_t*)mutex );
}

}
