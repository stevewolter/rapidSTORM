#ifndef DSTORM_MAINTHREAD_H
#define DSTORM_MAINTHREAD_H

#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <queue>
#include <boost/function/function0.hpp>
#include <boost/noncopyable.hpp>
#include <dStorm/JobMaster.h>
#include "InputStream.h"
#include <dStorm/stack_realign.h>

namespace dStorm {

namespace job { class Config; }

class MainThread
: public JobMaster {
public:
    MainThread();
    void run_all_jobs();
    std::auto_ptr<JobHandle> register_node( Job& );

    void connect_stdio( const job::Config& );
    void terminate_running_jobs();
private:
    boost::mutex mutex;
    boost::condition main_thread_wakeup, terminated_all_threads;
    std::set<Job*> active_jobs;
    int job_count;
    boost::shared_ptr<InputStream> io;
    boost::thread input_acquirer;
    boost::mutex::scoped_lock input_read_lock;

    DSTORM_REALIGN_STACK void read_input();
    void unregister_node( Job& job );
    void erase( Job& job );

    struct Handle : public JobHandle {
        MainThread& main_thread;
        Job& job;

        Handle( MainThread& main_thread, Job& job ) 
            : main_thread(main_thread), job(job) {}
        void unregister_node() { main_thread.unregister_node(job); }
        ~Handle() { main_thread.erase( job ); }
    };

};

}

#endif
