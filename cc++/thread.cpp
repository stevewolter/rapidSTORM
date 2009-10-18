/** \file thread.cpp
 *  Definition of the class methods found in the cc++/thread.h file.
 *
 *  For documentation, see cc++/thread.h
 **/
#define OST_EXITBLOCK_CPP
#include "thread.h"

#include <stdlib.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <iomanip>
#include <fstream>

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
    void DebugStream::begin(const char *file, int line) throw() { 
        LockedStream::begin();
        char *sl = strrchr(file, '/');
        base << right << setw(20) << ((sl != NULL) ? sl+1 : file) << ":"
             << left << setw(4) << line << " "
             << left << setw(9) << ost::Thread::description() << " "
             << left;
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

    /** This mutex controls access to \c allTerminated,
     *  \c detachedThreads and \c installedKey. */
    static pthread_mutex_t detachedThreadsMutex = 
                                          PTHREAD_MUTEX_INITIALIZER;
    /** This condition is signaled when the number of detached threads
     *  drops to 0, thus indicating that main may exit. */
    static pthread_cond_t allTerminated = PTHREAD_COND_INITIALIZER;
    /** This variable is locked by \c detachedThreadsMutex and counts
     *  the number of detached threads for this process. This number
     *  is used by Thread::joinDetached(). */
    static int detachedThreads = 0;
    /** Global variable indicating whether objKey was already 
     *  initialized. */
    static bool installedKey = false;
    static pthread_key_t objKey;

    void Thread::joinDetached() throw() {
        pthread_mutex_lock(&detachedThreadsMutex);
        while ( detachedThreads > 0 )
            pthread_cond_wait(&allTerminated, &detachedThreadsMutex);
        pthread_mutex_unlock(&detachedThreadsMutex);
    }

    void *Thread::runHandler(void *threadp) throw() {
        STATUS("Subthread started for " << threadp);
        Thread &thread = *(Thread*)threadp;
        pthread_cleanup_push( &callFinal, threadp );
        pthread_setspecific(objKey, &thread);
        thread.initial();
        PROGRESS("Running " << threadp);
        thread.run();
        PROGRESS("Ran " << threadp);
        pthread_cleanup_pop( 1 );
        STATUS("Finished subthread for " << threadp);
        return NULL;
    }

    void Thread::callFinal(void *threadp) throw() { 
        Thread &thread = *(Thread*)threadp;
        PROGRESS("Finalizing " << ((thread.detached) ? "detached " : "")
               << "thread " << threadp);
        thread.final(); 
        if ( thread.detached ) { 
            delete &thread;
            pthread_mutex_lock(&detachedThreadsMutex);
            detachedThreads--;
            if (detachedThreads == 0)
                pthread_cond_signal(&allTerminated);
            pthread_mutex_unlock(&detachedThreadsMutex);
        }
    }

    Thread::Thread(const char *name) throw() 
    : name(name)
    { 
        STATUS("Creating thread " << this);
        pthread_mutex_lock(&detachedThreadsMutex);
        if (! installedKey ) {
            pthread_key_create(&objKey, NULL);
            installedKey = true;
        }
        pthread_mutex_unlock(&detachedThreadsMutex);
        running = false; detached = false; 
    }
    void Thread::start() throw() {
        if (!running) {
            running = 
                (pthread_create(&thread, NULL, runHandler, this) == 0);
            if (!running) cerr << "Starting failed: " << errno << endl;
            detached = false;
        }
    }

    void Thread::detach() throw() {
        if (!running) { start(); }
        if (detached) return;
        pthread_detach(thread);
        detached = true;
        pthread_mutex_lock(&detachedThreadsMutex);
        detachedThreads++;
        pthread_mutex_unlock(&detachedThreadsMutex);
        PROGRESS("Detached " << this);
    }

    void Thread::join() throw() {
        if (running && !detached)  {
            STATUS("Joining thread " << this);
            running = false;
            pthread_join(thread, NULL); 
            STATUS("Joined thread " << this);
        }
    }

    const char *Thread::desc() const throw() {
        return name;
    }

    const char *Thread::description() throw() {
        const Thread *t = current_thread();
        if ( t != NULL )
            return t->desc();
        else
            return "NONE";
    }

    const ost::Thread *Thread::current_thread() throw() {
        if ( !installedKey ) return NULL;
        const ost::Thread* thread = 
            (const ost::Thread *)pthread_getspecific(objKey);
        return thread;
    }
}
