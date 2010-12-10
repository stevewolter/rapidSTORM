/** \file thread.cpp
 *  Definition of the class methods found in the cc++/thread.h file.
 *
 *  For documentation, see cc++/thread.h
 **/
#define OST_EXITBLOCK_CPP
#include "cc++/thread.h"

#include <stdlib.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <iomanip>

using namespace std;

namespace ost {

    DebugStream *DebugStream::globalDebugStream = NULL;

    /** This is the mutex for _lock_cerr() and _unlock_cerr(). */
    void DebugStream::begin(const char *file, int line) throw() { 
        LockedStream::begin();
        char *sl = strrchr(file, '/');
        base << right << setw(20) << ((sl != NULL) ? sl+1 : file) << ":"
             << left << setw(4) << line << " "
             << left << setw(9) << ost::Thread::description() << " "
             << left;
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
    /** Global variable indicating whether nameKey was already 
     *  initialized. */
    static bool installedKey = false;
    static pthread_key_t nameKey;

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
        pthread_setspecific(nameKey, thread.name);
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
            pthread_key_create(&nameKey, NULL);
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

    const char *Thread::description() throw() {
        if ( !installedKey ) return "NONE";
        const char *name = (const char *)pthread_getspecific(nameKey);
        return (name != NULL) ? name : "MAIN";
    }
}
