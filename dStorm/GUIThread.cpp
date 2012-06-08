#include "GUIThread.h"

namespace dStorm {

GUIThread& GUIThread::get_singleton() {
    static GUIThread* g = new GUIThread();
    return *g;
}

bool GUIThread::need_wx_widgets() {
    return ! tasks.empty();
}

void GUIThread::register_job( Job& job ) {
    boost::recursive_mutex::scoped_lock lock( mutex );
    ++job_count;
    active_jobs.insert( &job );
}

void GUIThread::unregister_job( Job& job ) {
    boost::lock_guard< boost::recursive_mutex > lock( mutex );
    active_jobs.erase( &job );
    --job_count;
    if ( job_count == 0 )
        have_task.notify_all();
}

void GUIThread::terminate_running_jobs() {
    boost::lock_guard< boost::recursive_mutex > lock( mutex );
    std::for_each( active_jobs.begin(), active_jobs.end(),
                   std::mem_fun(&Job::stop) );
}

int GUIThread::count_jobs() {
    boost::lock_guard< boost::recursive_mutex > lock( mutex );
    return job_count;
}

void GUIThread::register_unstopable_job() {
    boost::lock_guard< boost::recursive_mutex > lock( mutex );
    ++job_count;
}

void GUIThread::unregister_unstopable_job() {
    boost::lock_guard< boost::recursive_mutex > lock( mutex );
    --job_count;
    if ( job_count == 0 ) have_task.notify_all();
}

void GUIThread::run_all_jobs() 
{
    boost::recursive_mutex::scoped_lock lock( mutex );
    while ( job_count > 0 ) {
        have_task.wait( lock );
    }
}

void GUIThread::run_wx_function( Task t ) {
    tasks.push(t);
}

void GUIThread::perform_wx_tasks() {
    if ( recursive ) return;
    while (true) {
        Task t;
        {
            boost::recursive_mutex::scoped_lock lock( mutex );
            if ( tasks.empty() ) return;
            t = tasks.front();
            tasks.pop();
        }
        recursive = true;
        t();
        recursive = false;
    }
}

GUIThread::GUIThread() 
: job_count(0), recursive(false)
{
}

}
