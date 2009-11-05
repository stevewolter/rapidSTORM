#ifndef CCPP_RUNNABLES_H
#define CCPP_RUNNABLES_H

#include <dStorm/helpers/thread.h>
namespace ost {

/** A WaitableRunnable base class 
 *  allows the creating code to wait
 *  until the Runnable has been run in processing code.
 **/
class WaitableRunnable : public Runnable {
    Mutex mutex;
    bool did_run;
    Condition cond;
  public:
    /** Constructor. */
    WaitableRunnable() : did_run(false), cond(mutex) {}
    /** Wait until the WaitableRunnable has finished
     *  operation. */
    void wait() {
        MutexLock lock(mutex);
        while (!did_run) cond.wait();
    }
    /** The final() method is called by classes accepting
     *  Runnables when operations are finished. Remember
     *  to call this method if you extend the finish
     *  method. */
    void final() throw() { 
        MutexLock lock(mutex);
        did_run = true;  
        cond.signal();
    }
};

}

#endif
