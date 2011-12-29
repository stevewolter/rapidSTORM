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

}
