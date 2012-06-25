#include <wx/app.h>
#include "GUIThread.h"
#include <iostream>

namespace dStorm {

bool GUIThread::Task::operator<( const Task& o ) const {
    return ! ( (priority != o.priority) ? priority < o.priority : sequence < o.sequence );
}

GUIThread& GUIThread::get_singleton() {
    static GUIThread* g = new GUIThread();
    return *g;
}

bool GUIThread::need_wx_widgets() {
    return ! tasks.empty();
}

void GUIThread::register_job( Job& job ) {
    boost::recursive_mutex::scoped_lock lock( mutex );
    active_jobs.insert( &job );
}

void GUIThread::unregister_job( Job& job ) {
    boost::lock_guard< boost::recursive_mutex > lock( mutex );
    active_jobs.erase( &job );
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
    boost::recursive_mutex::scoped_lock lock( mutex );
    if ( sequence_number == 0 ) wxWakeUpIdle();
    t.sequence = sequence_number++;
    tasks.push(t);
}

void GUIThread::perform_wx_tasks() {
    if ( recursive ) return;
    while (true) {
        boost::recursive_mutex::scoped_lock lock( mutex );
        if ( tasks.empty() ) { sequence_number = 0; return; }
        Task t = tasks.top();
        tasks.pop();
        lock.unlock();
        recursive = true;
        try {
            t.f();
        } catch ( const std::exception& e ) {
            std::cerr << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Unknown error in perform_wx_tasks" << std::endl;
        }
        recursive = false;
    }
}

GUIThread::GUIThread() 
: job_count(0), recursive(false), sequence_number(0)
{
}

}
