#include <wx/app.h>
#include "GUIThread.h"
#include <iostream>
#include <boost/optional/optional.hpp>

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
    return active_jobs.size();
}

void GUIThread::wait_for_thread( std::auto_ptr<boost::thread> t ) {
    boost::lock_guard< boost::recursive_mutex > lock( mutex );
    active_threads.insert( std::make_pair( t->get_id(), t.get() ) );
    /* The thread is now implicitly managed by this class and will
     * be deleted once unregister_thread() is called. */
    t.release();
}

void GUIThread::join_this_thread() {
    boost::lock_guard< boost::recursive_mutex > lock( mutex );
    joinable_threads.push( boost::this_thread::get_id() );
    have_task.notify_all();
}

void GUIThread::join_joinable_threads( boost::recursive_mutex::scoped_lock& lock ) {
    while ( ! joinable_threads.empty() ) {
        {
            /* Careful handling of this ID is necessary. At least for 
             * POSIX threads implementation, the ID keeps a handle on the
             * thread open. */
            boost::optional<boost::thread::id> next_join = joinable_threads.front();
            joinable_threads.pop();
            boost::thread* thread = active_threads[ *next_join ];
            active_threads.erase( *next_join );
            lock.unlock();
            next_join.reset();
            thread->join();
            delete thread;
            lock.lock();
        }
    }
}

void GUIThread::run_all_jobs() 
{
    boost::recursive_mutex::scoped_lock lock( mutex );
    while ( ! active_threads.empty() ) {
        join_joinable_threads( lock );
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
        if ( tasks.empty() ) { sequence_number = 0; break; }
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

    boost::recursive_mutex::scoped_lock lock( mutex );
    join_joinable_threads( lock );
}

GUIThread::GUIThread() 
: recursive(false), sequence_number(0)
{
}

}
