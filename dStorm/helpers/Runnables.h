#ifndef CCPP_RUNNABLES_H
#define CCPP_RUNNABLES_H

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/utility.hpp>

namespace dStorm {

template <typename Functor>
struct Waitable 
{
    struct run_result {
        boost::mutex mutex;
        bool did_run;
        boost::condition_variable cond;
        run_result() : did_run(false) {}
    };
    boost::shared_ptr<run_result> r;

    Functor _functor;
  public:
    /** Constructor. */
    Waitable( Functor& o ) : r(new run_result()), _functor(o) {}
    Waitable( const Functor& o ) : r(new run_result()), _functor(o) {}
    Waitable( Waitable& o ) : r(o.r), _functor(o._functor) {}
    Waitable( const Waitable& o ) : r(o.r), _functor(o._functor) {}
    template <typename Type1>
    Waitable(Type1& t1) : r(new run_result()), _functor(t1) {}
    template <typename Type1, typename Type2>
    Waitable(Type1& t1, Type2& t2) : r(new run_result()), _functor(t1,t2) {}
    template <typename Type1, typename Type2, typename Type3>
    Waitable(Type1& t1, Type2& t2, Type3& t3) : r(new run_result()), _functor(t1,t2,t3) {}
    /** Wait until the WaitableRunnable has finished
     *  operation. */
    void wait() {
        boost::unique_lock<boost::mutex> lock(r->mutex);
        while (!r->did_run) r->cond.wait(lock);
    }
    /** The final() method is called by classes accepting
     *  Runnables when operations are finished. Remember
     *  to call this method if you extend the finish
     *  method. */
    void operator()() { 
        _functor();
        boost::unique_lock<boost::mutex> lock(r->mutex);
        r->did_run = true;  
        r->cond.notify_all();
    }

    Functor& functor() { return _functor; }
    const Functor& functor() const { return _functor; }
};

template <class Functor>
Waitable<Functor> make_waitable( const Functor& f ) { 
    return Waitable<Functor>(f);
}

}

#endif
