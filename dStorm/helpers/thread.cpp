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

using namespace std;

namespace ost {
    DebugStream *DebugStream::globalDebugStream = NULL;

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
        return globalDebugStream;
    }
    void DebugStream::set(std::ostream &o) throw() { 
        if (globalDebugStream) delete globalDebugStream;
        globalDebugStream = new DebugStream(o);
    }

}
