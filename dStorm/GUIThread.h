#ifndef DSTORM_MAINTHREAD_H
#define DSTORM_MAINTHREAD_H

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>
#include <dStorm/Job.h>
#include <queue>
#include <map>
#include <set>
#include <boost/function/function0.hpp>

namespace dStorm {

class GUIThread 
{
public:
    static GUIThread& create_singleton(char* program_name);
    static GUIThread& get_singleton();

    void perform_wx_tasks();
    void run_wx_gui_thread();

    void register_job( Job& );
    void unregister_job( Job& );
    void wait_for_thread( std::auto_ptr<boost::thread> );
    void join_this_thread();
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
    std::map< boost::thread::id, boost::thread* > active_threads;
    std::queue< boost::thread::id > joinable_threads;

    boost::thread thread_collector;

    bool recursive;
    int sequence_number;
    char* const prog_name;
    bool wx_widgets_running;

    void join_joinable_threads( boost::recursive_mutex::scoped_lock& );
    void wait_for_threads();

    static GUIThread* singleton;

    GUIThread(char* prog_name);
    ~GUIThread() {}
};

}

#endif
