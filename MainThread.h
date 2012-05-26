#ifndef DSTORM_MAINTHREAD_H
#define DSTORM_MAINTHREAD_H

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <dStorm/Job.h>
#include <set>

namespace dStorm {

class MainThread 
{
public:
    MainThread();
    void run_all_jobs();

    void register_job( Job& );
    void unregister_job( Job& );

    void register_unstopable_job();
    void unregister_unstopable_job();

    void terminate_running_jobs();

    int count_jobs();
private:
    boost::mutex mutex;
    boost::condition main_thread_wakeup;
    std::set<Job*> active_jobs;
    int job_count;
};

}

#endif
