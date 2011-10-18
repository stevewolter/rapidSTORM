/** \file thread.cpp
 *  Definition of the class methods found in the cc++/thread.h file.
 *
 *  For documentation, see cc++/thread.h
 **/
#define OST_EXITBLOCK_CPP
#include "thread.h"

#include <boost/thread/thread.hpp>
#include <stdlib.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <iomanip>
#include <fstream>
#include <stdexcept>
#include <sys/time.h>
#include <semaphore.h>

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
    static void printDebugHeader(std::ostream& base, const char *file, int line) throw() { 
        const char *sl = strrchr(file, '/');
        base << right << setw(20) << ((sl != NULL) ? sl+1 : file) << ":"
             << left << setw(4) << line << " "
             << left << setw(9) << boost::this_thread::get_id() << " "
             << left;
    }
    void DebugStream::begin(const char *file, int line) throw() 
    { 
        LockedStream::begin();
        printDebugHeader(base, file, line);
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

    void Condition::timed_wait(int milliseconds) throw() {
        struct timeval now;
        struct timespec timeout;

        gettimeofday(&now, NULL);
        timeout.tv_sec = now.tv_sec;
        timeout.tv_nsec = now.tv_usec * 1000;
        while ( milliseconds > 0 ) {
            timeout.tv_nsec += 1E6;
            milliseconds -= 1;
            if ( timeout.tv_nsec >= 1E9 ) {
                timeout.tv_sec += 1;
                timeout.tv_nsec -= 1E9;
            }
        }
        int rv = pthread_cond_timedwait(&cond, &mutex.mutex, &timeout);
        if ( rv == ETIMEDOUT )
            return;
        else
            return;
    }

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
    }
    WriteLock::~WriteLock() throw() {
        tl.waitingWriters--;
        if (tl.waitingWriters == 0)
            pthread_cond_broadcast(&tl.admitReader);
        else
            pthread_cond_signal(&tl.canWrite);
        pthread_mutex_unlock(&tl.mutex);
    }

}
