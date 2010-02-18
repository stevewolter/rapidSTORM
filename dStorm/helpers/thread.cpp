/** \file thread.cpp
 *  Definition of the class methods found in the cc++/thread.h file.
 *
 *  For documentation, see cc++/thread.h
 **/
#define OST_EXITBLOCK_CPP
#define VERBOSE
#include "thread.h"

#include <stdlib.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <iomanip>
#include <fstream>
#include <stdexcept>
#include <signal.h>

#define VERBOSE
#include "../debug.h"

using namespace std;

namespace ost {
#if 0
    static pthread_key_t *debug_stream_key = NULL;
    static void destructDebugStream(void *str) throw()
        { delete (DebugStream*)str; }
#else
    DebugStream *DebugStream::globalDebugStream = NULL;
#endif

    DebugStream::DebugStream(std::ostream &target) throw() : LockedStream(target) {}

    /** This is the mutex for _lock_cerr() and _unlock_cerr(). */
    static void printDebugHeader(std::ostream& base, const char *file, int line, const char *thread) throw() { 
        char *sl = strrchr(file, '/');
        base << right << setw(20) << ((sl != NULL) ? sl+1 : file) << ":"
             << left << setw(4) << line << " "
             << left << setw(9) << thread << " "
             << left;
    }
    void DebugStream::begin(const char *file, int line) throw() 
    { 
        LockedStream::begin();
        printDebugHeader(base, file, line, dStorm::Thread::description());
    }
    void DebugStream::end() throw() { LockedStream::end(); }

    DebugStream* DebugStream::get() throw() {
#if 0
        if (debug_stream_key == NULL) {
            debug_stream_key = new pthread_key_t;
            pthread_key_create(debug_stream_key, destructDebugStream);
        }
        void *specific = pthread_getspecific(*debug_stream_key);
        if (specific == NULL) {
            string name = "debug_" + string(Thread::description());
            specific = new DebugStream(*new fstream(name.c_str()
                , ios_base::out | ios_base::trunc));
            pthread_setspecific(*debug_stream_key, specific);
        }
        return (DebugStream*)specific;
#else
        return globalDebugStream;
#endif
    }
    void DebugStream::set(std::ostream &o) throw() { 
        if (globalDebugStream) delete globalDebugStream;
        globalDebugStream = new DebugStream(o);
    }

    Mutex::Mutex() throw() {
        /* This is unportable, but widely recognized. */
        pthread_mutexattr_t attributes;
        pthread_mutexattr_init(&attributes);
        pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_RECURSIVE_NP);
        pthread_mutex_init(&mutex, &attributes);
    }

    Mutex::~Mutex() throw() { pthread_mutex_destroy(&mutex); }

    ThreadLock::ThreadLock() throw() : readers(0), waitingWriters(0) {
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&canWrite, NULL);
        pthread_cond_init(&admitReader, NULL);
    }

    ThreadLock::~ThreadLock() throw() {
        pthread_cond_destroy(&canWrite);
        pthread_cond_destroy(&admitReader);
        pthread_mutex_destroy(&mutex);
    }

    ReadLock::ReadLock(ThreadLock &tl) throw() : tl(tl) {
        pthread_mutex_lock(&tl.mutex);
        while (tl.waitingWriters > 0)
            pthread_cond_wait(&tl.admitReader, &tl.mutex);
        tl.readers++;
        pthread_mutex_unlock(&tl.mutex);
    }
    ReadLock::~ReadLock() throw() {
        pthread_mutex_lock(&tl.mutex);
        tl.readers--;
        if (tl.readers == 0)
            pthread_cond_signal(&tl.canWrite);
        pthread_mutex_unlock(&tl.mutex);
    }

    WriteLock::WriteLock(ThreadLock &tl) throw() : tl(tl) {
        /* This mutex is kept until the write operation is finished. */
        pthread_mutex_lock(&tl.mutex);
        tl.waitingWriters++;
        while (tl.readers > 0)
            pthread_cond_wait(&tl.canWrite, &tl.mutex);
        tl.waitingWriters--;
    }
    WriteLock::WriteLock(ReadLock &rl) throw() : tl(rl.tl) {
        /* This mutex is kept until the write operation is finished. */
        pthread_mutex_lock(&tl.mutex);
        tl.waitingWriters++;
        while (tl.readers > 1)          /* The 1 is for the rl ReadLock */
            pthread_cond_wait(&tl.canWrite, &tl.mutex);
        tl.waitingWriters--;
    }
    WriteLock::~WriteLock() throw() {
        if (tl.waitingWriters == 0)
            pthread_cond_signal(&tl.admitReader);
        pthread_mutex_unlock(&tl.mutex);
    }

}

#include <boost/thread/thread.hpp>

namespace dStorm {

    /** Global variable indicating whether objKey was already 
     *  initialized. */
    static bool installedKey = false;
    static pthread_key_t objKey;
    static pthread_mutex_t static_mutex = PTHREAD_MUTEX_INITIALIZER;
    static pthread_cond_t no_detached_threads = PTHREAD_COND_INITIALIZER;
    int detached_threads = 0;

    static dStorm::Runnable* thread_init = NULL;

Runnable* Thread::set_thread_initializer( Runnable& r ) {
    Runnable *o = thread_init;
    thread_init = &r;
    return o;
}

struct Thread::Pimpl {
    Thread& object;
    /** Indicates whether a subthread is running, detached or not. */
    bool running;
    /** Indicates whether the running subthread is detached. */
    bool detached;
    /** Indicates whether a detached subthread should delete its own thread
     *  class. This is set apart from detached to ensure that emergency
     *  cleanup does not delete the thread class. */
    bool delete_self;
    /** Handle for the running subthread. Only valid for running == true */
    pthread_t thread;
    /** The name of the created subthread that is returned by 
     *  description() */
    const char * const name;
#ifdef PTW32_VERSION
    /** Mutex for \c stopped_execution. */
    ost::Mutex execution_status_mutex;
    /** Pthread-win32 fix for pthread_join with condition variable.
     *  Pthread-win32 sometimes hangs (under unknown circumstances) while
     *  trying to join a thread. We circumvent this by emulating join with
     *  this condition, which is protected by the mutex and the condition
     *  variable. */
    ost::Condition stopped_execution;
#endif

    /** This function gets called for cleanup uf the cleanupKey entries */
    static void cleanUp(void* threadp);
    /** This function gets called by pthread_create() */
    static void* runHandler(void *threadp) throw()
         __attribute__((force_align_arg_pointer));

    Pimpl(Thread& obj, const char *name);
    void fork();
};

void *Thread::Pimpl::runHandler(void *threadp) throw() {
    DEBUG("Subthread started for " << threadp);
    Thread &thread = *(Thread*)threadp;
    pthread_setspecific(objKey, &thread);

    if ( thread_init != NULL ) {
        DEBUG("Using thread initializer");
        thread_init->run();
    } else {
        DEBUG("No thread initializer defined");
    }

#if 0
    /* Block termination signals; these are handled by the 
     * main thread. */
    sigset_t notermsigs;
    sigemptyset(&notermsigs);
    sigaddset(&notermsigs, SIGHUP);
    sigaddset(&notermsigs, SIGTERM);
    sigaddset(&notermsigs, SIGINT);
    sigaddset(&notermsigs, SIGQUIT);
    pthread_sigmask( SIG_BLOCK, &notermsigs, NULL );
#endif
    
    DEBUG("Running " << threadp);
    try {
        thread.run();
    } catch ( const std::exception& e ) {
        std::cerr << e.what() << "\n";
    }
    if ( thread.pimpl->detached )
        thread.pimpl->delete_self = true;
    DEBUG("Finished subthread for " << threadp);
    return NULL;
}

void Runnable::abnormal_termination(std::string r) {
    Thread *t = dynamic_cast<Thread*>(this);
    if ( t != NULL ) {
        std::cerr << "Terminating thread " 
                  << t->description() 
                  << ": " << r << std::endl;
    }
    pthread_exit( NULL );
}

Thread::Pimpl::Pimpl(Thread& obj, const char *name) 
: object(obj), running(false), detached(false), delete_self(false),
  name(name)
#ifdef PTW32_VERSION
  , stopped_execution(execution_status_mutex)
#endif
{}

Thread::Thread(const char *name) throw() 
: pimpl( new Pimpl(*this, name) )
{ 
    DEBUG("Creating thread " << this);
    if (! installedKey ) {
        installedKey = true;
        pthread_key_create(&objKey, Pimpl::cleanUp);
    }
}

Thread::~Thread() { join(); }

void Thread::Pimpl::fork()  {
    if (!running) {
        DEBUG("Starting thread " << this);
        running = (pthread_create(&thread, NULL, runHandler, &object) == 0);
        if (!running)
            throw std::runtime_error("Starting thread failed.");
        else
            DEBUG("Started thread successfully");
    }
}

void Thread::start() {
    pimpl->detached = false;
    pimpl->fork();
}

void Thread::detach() throw() {
    DEBUG("Detaching " << this);
    pimpl->detached = true;
    pimpl->fork();
    pthread_detach(pimpl->thread);
    
    DEBUG("Updating detached thread count");
    pthread_mutex_lock(&static_mutex);
    detached_threads++;
    pthread_mutex_unlock(&static_mutex);
    DEBUG("Detached " << this << ", now " << detached_threads <<
          "detached threads");
}

void Thread::join() throw() {
    DEBUG("Calling join on thread " << this);
    if (pimpl->running && !pimpl->detached)  {
        DEBUG("Joining thread " << this << "from " << current_thread());
#if 0
        DEBUG("Waiting for thread finish mutex for " << this);
        ost::MutexLock lock( execution_status_mutex );
        DEBUG("Got thread finish mutex for" << this);
        while ( running ) {
            DEBUG("Waiting for execution to stop for " << this);
            stopped_execution.wait();
        }
#else
        pimpl->running = false;
        pthread_join(pimpl->thread, NULL); 
#endif
        DEBUG("Joined thread " << this);
    }
}

const char *Thread::desc() const throw() {
    return pimpl->name;
}

const char *Thread::description() throw() {
    const Thread *t = current_thread();
    if ( t != NULL )
        return t->desc();
    else
        return "NONE";
}

Thread *Thread::current_thread() throw() {
    if ( !installedKey ) return NULL;
    Thread* thread = 
        (Thread *)pthread_getspecific(objKey);
    return thread;
}

void Thread::exit() {
    pthread_exit( NULL );
}

void Thread::wait_for_detached_threads() {
    pthread_mutex_lock( &static_mutex );
    while ( detached_threads > 0 )
        pthread_cond_wait( 
            &no_detached_threads,
            &static_mutex );
    pthread_mutex_unlock( &static_mutex );
}

static std::ostream& threadlessMessage(int line)
{
    ost::DebugStream::get()->LockedStream::begin();
    ost::printDebugHeader( *ost::DebugStream::get(), __FILE__, line, "");
    return *ost::DebugStream::get();
}

void Thread::Pimpl::cleanUp(void *data) {
#ifdef VERBOSE
    threadlessMessage(__LINE__)
        << "Cleaning up data for " << data << std::endl;
    ost::DebugStream::get()->end();
#endif
    Thread* t = (Thread*)data;
    bool detached = t->pimpl->detached;
    if ( t->pimpl->delete_self )
        delete t;
    if ( detached ) {
        pthread_mutex_lock( &static_mutex );
        detached_threads--;
        if ( detached_threads == 0 )
            pthread_cond_signal( &no_detached_threads );
#ifdef VERBOSE
        threadlessMessage(__LINE__) <<
        "Reducing count of detached threads to " << detached_threads
            << std::endl;
        ost::DebugStream::get()->end();
#endif
        pthread_mutex_unlock( &static_mutex );
    }
}


}
