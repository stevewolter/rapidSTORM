#include "debug.h"
#include "MainThread.h"
#include <iostream>
#include <ios>

namespace dStorm {

MainThread::MainThread() 
: job_count(0)
{
}

void MainThread::run_all_jobs() 
{
    boost::mutex::scoped_lock lock( mutex );
    while ( job_count > 0 ) {
        main_thread_wakeup.wait(lock);
    }
}

void MainThread::register_job( Job& job ) {
    boost::mutex::scoped_lock lock( mutex );
    ++job_count;
    active_jobs.insert( &job );
}

void MainThread::unregister_job( Job& job ) {
    boost::lock_guard< boost::mutex > lock( mutex );
    active_jobs.erase( &job );
    --job_count;
    if ( job_count == 0 )
        main_thread_wakeup.notify_all();
}

void MainThread::terminate_running_jobs() {
    boost::lock_guard< boost::mutex > lock( mutex );
    DEBUG("Terminate remaining cars");
    std::for_each( active_jobs.begin(), active_jobs.end(),
                   std::mem_fun(&Job::stop) );
}

int MainThread::count_jobs() {
    boost::lock_guard< boost::mutex > lock( mutex );
    return job_count;
}

void MainThread::register_unstopable_job() {
    boost::lock_guard< boost::mutex > lock( mutex );
    ++job_count;
}

void MainThread::unregister_unstopable_job() {
    boost::lock_guard< boost::mutex > lock( mutex );
    --job_count;
    if ( job_count == 0 ) main_thread_wakeup.notify_all();
}

}
