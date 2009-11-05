#ifndef OST_THREAD_H
#define OST_THREAD_H

#include <sys/types.h>
#include <pthread.h>
#include <iostream>

/** The ost namespace provides C++ thread locking mechanisms.
 *  It exports the same public interface as the GNU CommonC++ library.
 *  This re-implementation is necessary because that library proved
 *  faulty under MS Windows. */
namespace ost {

/** The Mutex class provides a recursive mutual exclusion lock.
 *  This lock may be locked in the old-fashioned way by enterMutex()
 *  and leaveMutex(), which are the equivalents of pthread_mutex_lock()
 *  and pthread_mutex_unlock(), or with a MutexLock object. */
class Mutex {
  private:
    /** Underlying pthread mutex. */
    pthread_mutex_t mutex;
    friend class Condition;
    friend class MutexLock;
  public:
    /** Constructor. The mutex will not be locked on construction. */
    Mutex() throw();
    /** Destructor. A mutex that is destructed should be in the unlocked
     *  state. */
    ~Mutex() throw();
    /** Lock the mutex. */
    inline void enterMutex() throw() { pthread_mutex_lock(&mutex); }
    /** Unlock the mutex. */
    inline void leaveMutex() throw() { pthread_mutex_unlock(&mutex); }
};

/** The MutexLock object holds a lock on a provided mutex during
 *  its lifetime. It acquires the lock on
 *  construction and releases it on destruction. Use this class to
 *  exception-safely lock a Mutex. Typical use is
 *
 *  \code
 *  MutexLock lock(mutex);
 *  \endcode
 *
 *  which locks the mutex until the end of the current block.
 *  */
class MutexLock {
  private:
    /** Reference to the underlying mutex */
    pthread_mutex_t &mutex;

  public:
    /** Constructor A: With an ost::Mutex */
    inline MutexLock(Mutex &mutex) throw() : mutex(mutex.mutex) 
        { pthread_mutex_lock(&this->mutex); }
    /** Constructor b: With pthread_mutex_t for backwards compatibility */
    inline MutexLock(pthread_mutex_t &mutex) throw() : mutex(mutex) 
        { pthread_mutex_lock(&this->mutex); }
    /** The destructor unlocks the underlying mutex. */
    inline ~MutexLock() throw() { pthread_mutex_unlock(&mutex); }
};

/** The Condition class implements a condition variable. This is a
 *  synchronization device that allows threads to suspend execution
 *  with the wait() method until a certain condition is met, which is 
 *  signaled by the signal() method.
 *
 *  Condition variables operate on an underlying mutex (called the mutex
 *  in the description) that should
 *  control access to the condition that is to be met. 
 **/
class Condition {
  private:
    /** The underlying mutex. */
    Mutex& mutex;
    /** pthread structure for condition variables. */
    pthread_cond_t cond;

  public:
    /** The Condition constructor initializes variables, but does not
     *  block for any reason. */
    Condition(Mutex &mutex) throw() : mutex(mutex) {
        pthread_cond_init(&cond, NULL);
    }

    /** The Condition destructor frees the condition object. It assumes
     *  that no threads are currently waiting on the condition variable. */
    ~Condition() throw() {
        pthread_cond_destroy(&cond);
    }

    /** This method restarts one of the threads waiting for this 
     *  Condition. The calling thread may or may not hold the mutex;
     *  if it does, remember that the restarted thread locks until the
     *  mutex is released.
     *
     *  If no threads are waiting for this condition, this method
     *  does nothing. */
    void signal() throw() { pthread_cond_signal(&cond); }
    /** Like signal(), but signals all waiting threads. */
    void broadcast() throw() { pthread_cond_broadcast(&cond); }
    /** This method blocks the calling thread until signal() or 
     *  broadcast() is called. The calling thread is assumed to
     *  have locked the underlying mutex; it is atomically unlocked
     *  on call to wait() and locked before wait() returns. */
    void wait() throw() { pthread_cond_wait(&cond, &mutex.mutex); }
};

/** A ThreadLock provides a reader-writer protection scheme.
 *  On an object of the ThreadLock type one may acquire two different
 *  kinds of locks: ReadLock and WriteLock. 
 * 
 *  At any given time, an unlimited number of ReadLock objects OR a single
 *  WriteLock is allowed. Thus, a thread holding one of the two kinds
 *  of locks is asserted that no other thread currently holds a WriteLock
 *  on the object; if a thread holds a WriteLock, it is asserted
 *  that no other thread holds any kind of lock.
 *
 *  The implementation of this class priotizes writer access. Thus, if
 *  any writers are waiting for a WriteLock, no readers are admitted.
 */
class ThreadLock {
  private:
    /** Number of hold ReadLock objects. WriteLock objects may not
     *  acquire a lock until this number is 0, which is indicated
     *  by the \c canWrite condition. */
    int readers;
    /** Number of WriteLock objects waiting for a WriteLock. Until
     *  this number is 0, no ReadLock is admitted; admittance is indicated
     *  by \c admitReader. */
    int waitingWriters;
    /** Mutex for any changes on this object. A WriteLock will hold 
     *  this mutex until it is destroyed. */
    pthread_mutex_t mutex;
    /** This condition is signaled whenever the number of readers drops
     *  to 0, indicating writers may acquire locks on this object. */
    pthread_cond_t canWrite;
    /** This condition is signaled whenever the number of writers drops
     *  to 0, indicating readers may acquire a ReadLock on this object. */
    pthread_cond_t admitReader;

    friend class ReadLock;
    friend class WriteLock;

  public:
    /** Constructor: Doesn't do much. */
    ThreadLock() throw();
    ~ThreadLock() throw();
};

/** A ReadLock is a shared lock on a ThreadLock object that allows 
 *  other ReadLock objects to lock the ThreadLock concurrently,
 *  but prevents WriteLock objects to be acquired on the ThreadLock.
 **/
class ReadLock {
  private:
    friend class WriteLock;
    /** ThreadLock object that is locked. */
    ThreadLock &tl;
  public:
    /** The ReadLock constructor blocks the ThreadLock until it 
     *  acquires a reading lock. */
    ReadLock(ThreadLock &tl) throw();
    /** The ReadLock destructor releases the lock. */
    ~ReadLock() throw();
};

/** A WriteLock is a shared lock on a ThreadLock object that guarantees
 *  exclusive access.
 *
 *  WriteLocks do not support recursive use; thus, any process acquiring
 *  a WriteLock on an object when it already possesses one deadlocks. */
class WriteLock {
  private:
    /** ThreadLock object that is locked. */
    ThreadLock &tl;
  public:
    /** The WriteLock constructor blocks the ThreadLock until it 
     *  acquires a write lock. */
    WriteLock(ThreadLock &tl) throw();
    /** This WriteLock constructor upgrades a ReadLock to a WriteLock.
     *  The ReadLock reference will not be kept; destruction of the two
     *  locks is allowed in any order. */
    WriteLock(ReadLock &readLock) throw();
    /** The WriteLock destructor releases the lock. */
    ~WriteLock() throw();
};

/** A Runnable is a generalization of a function called
 *  independently of any context. It can be run 
 *  concurrently by a Thread object or by other methods.
 *  When calling Runnable objects, make sure to call all
 *  three methods in sequence.
 */
struct Runnable {
    virtual ~Runnable() {}

    /** This method is called just prior to run()
     *  and should be used for any necessary
     *  initialization. */
    virtual void initial() throw() {}
    /** This method is called as the main code of the
     *  runnable execution. */
    virtual void run() throw() = 0;
    /** This method must be called after the run() method
     *  of the runnable exits, no matter how the run()
     *  method exited. */
    virtual void final() throw() {}
};

/** A Thread object allows objects to execute one function concurrently.
 *  
 *  A Thread object will start concurrent execution of its run() function
 *  when start() or detach() is called. The new thread will begin execution
 *  of the run() method immediately and run concurrently to the thread
 *  calling start() or detach().
 *
 *  The concurrent thread will run until exiting from its run() method.
 *  The calling thread may wait on the termination of the concurrent
 *  thread by calling join() or the thread destructor. The calling thread
 *  is responsible for the deallocation of the thread object. The 
 *  concurrent thread will stay allocated until the calling thread
 *  joins it.
 *
 *  If the thread is detached, either by starting it with detach() or
 *  by calling detach() from either thread after starting with start(), 
 *  the thread is put in detached state. A thread in detached state is
 *  an independent object. When its run() method terminates, it stops
 *  execution immediately and frees all resources. This includes the
 *  Thread object itself, which is deleted after run() terminates.
 *  To repeat the previous statement: It is NOT the calling thread's 
 *  responsibility to deallocate a concurrent and detached thread. It
 *  does this by itself. Thus, calls like
 *  \code
 *  (new Thread())->detach();
 *  \endcode
 *  are perfectly valid.
 **/
class Thread : protected Runnable {
  private:
    /** Indicates whether a subthread is running, detached or not. */
    bool running;
    /** Indicates whether the running subthread is detached. */
    bool detached;
    /** Handle for the running subthread. Only valid for running == true */
    pthread_t thread;
    /** The name of the created subthread that is returned by 
     *  description() */
    const char * const name;
#ifdef PTW32_VERSION
    /** Mutex for \c stopped_execution. */
    Mutex execution_status_mutex;
    /** Pthread-win32 fix for pthread_join with condition variable.
     *  Pthread-win32 sometimes hangs (under unknown circumstances) while
     *  trying to join a thread. We circumvent this by emulating join with
     *  this condition, which is protected by the mutex and the condition
     *  variable. */
    Condition stopped_execution;
#endif

    /** This function gets called by pthread_create() */
    static void* runHandler(void *threadp) throw()
         __attribute__((force_align_arg_pointer));
    /** This is a cleanup handler called if anything untoward happens
     *  to the subthread. */
    static void callFinal(void *threadp) throw();

  public:
    /** Construct a thread with a description string that can be
     *  retrieved with description(). */
    Thread(const char *name) throw();
    /** The destructor of a Thread object that still has a running and
     *  joinable subthread waits until that subthread terminates. */
    virtual ~Thread() { join(); }

    /** Start the concurrent thread in a joinable state.
     *  Does nothing if there already is a subthread running. */
    void start(); 
    /** Start the concurrent thread in detached state or detach
     *  the running and joinable subthread, depending on the existence of 
     *  a subthread. */
    void detach() throw(); 
    /** Join the concurrent, joinable thread. Does nothing if the thread
     *  wasn't started or was detached. */
    void join() throw();

    /** Block until ALL detached subthreads terminated. Useful as the
     *  last action of the main() function that allows all detached
     *  subthreads to finish execution. */
    static void joinDetached() throw();
    /** Return the description string of the thread
     *  given in the constructor. */
    const char *desc() const throw();
    /** Return the description string of the Thread object
     *  that started the thread the code is running in,
     *  or "NONE" if the thread was not started by this
     *  library. */
    static const char *description() throw();
    /** Return a pointer to the Thread object that started
     *  the thread the code is running in.
     *  @return A pointer to the current thread object,
     *          or NULL if the current thread was not
     *          started by this library.
     **/
    static const Thread *current_thread() throw();
};

/** The LockedStream class provides concurrent output onto a stream.
 *  The calling thread is guaranteed exclusive access until the 
 *  */
class LockedStream {
  protected:
    Mutex mutex;
    std::ostream &base;
  public:
    LockedStream(std::ostream &target) throw() : base(target) {}

    operator std::ostream&() throw() { return base; }
    void begin() throw() { mutex.enterMutex(); }
    void end() throw() { mutex.leaveMutex(); }
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
