#ifndef OST_THREAD_H
#define OST_THREAD_H

#include <sys/types.h>
#include <boost/thread/mutex.hpp>
#include <iostream>
#include <memory>

/** The ost namespace provides C++ thread locking mechanisms.
 *  It exports the same public interface as the GNU CommonC++ library.
 *  This re-implementation is necessary because that library proved
 *  faulty under MS Windows. */
namespace ost {

/** The LockedStream class provides concurrent output onto a stream.
 *  The calling thread is guaranteed exclusive access until the 
 *  */
class LockedStream {
  protected:
    boost::mutex mutex;
    std::ostream &base;
  public:
    LockedStream(std::ostream &target) throw() : base(target) {}

    operator std::ostream&() throw() { return base; }
    void begin() throw() { mutex.lock(); }
    void end() throw() { mutex.unlock(); }
};

/** The DebugStream class is a DebugStream that prepends each output with
 *  information about the file, line and thread the output occured in.
 *  In addition, the class provides a global variable through which the 
 *  preferred stream for debug output can be set. */
class DebugStream : public LockedStream {
  private:
    static DebugStream *globalDebugStream;
  public:
    DebugStream(std::ostream &target) throw();
    void begin(const char *filename, int line) throw();
    void end() throw();

    static DebugStream* get() throw();
    static void set(std::ostream &o) throw(); 
};


}

#endif
