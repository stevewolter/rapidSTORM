#ifndef DSTORM_MAINTHREAD_H
#define DSTORM_MAINTHREAD_H

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition.hpp>
#include <dStorm/Job.h>
#include <queue>
#include <set>
#include <boost/function/function0.hpp>

namespace dStorm {

class GUIThread 
{
public:
    static GUIThread& get_singleton();

    bool need_wx_widgets();

    void perform_wx_tasks();
    void run_all_jobs();

    void register_job( Job& );
    void unregister_job( Job& );
    void register_unstopable_job();
    void unregister_unstopable_job();
    void terminate_running_jobs();
    int count_jobs();

    struct Task {
        boost::function0<void> f;
        int sequence, priority;
        bool operator<( const Task& o ) const;
        Task( boost::function0<void> f, int priority ) : f(f), priority(priority) {}
    };
    void run_wx_function( Task );
private:
    boost::recursive_mutex mutex;
    boost::condition have_task;
    std::priority_queue< Task > tasks;
    std::set<Job*> active_jobs;
    int job_count;
    bool recursive;
    int sequence_number;

    GUIThread();
    ~GUIThread() {}
};

}

#endif
