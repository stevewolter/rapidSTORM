#include "debug.h"
#include "MainThread.h"
#include <iostream>
#include <ios>

namespace dStorm {

MainThread::MainThread() 
: job_count(0),
  io( new InputStream(*this) ),
  input_read_lock(mutex)
{
    input_read_lock.unlock();
}

void MainThread::run_all_jobs() 
{
    boost::mutex::scoped_lock lock( mutex );
    while ( job_count > 0 ) {
        main_thread_wakeup.wait(mutex);
    }
}

std::auto_ptr<JobHandle> MainThread::register_node( Job& job ) {
    boost::mutex::scoped_lock lock( mutex );
    job.attach_ui( io );
    ++job_count;
    active_jobs.insert( &job );
    return std::auto_ptr<JobHandle>( new Handle( *this, job ) );
}

void MainThread::unregister_node( Job& job ) {
    boost::mutex::scoped_lock lock( mutex );
    job.detach_ui( io->get_handle() );
    active_jobs.erase( &job );
    if ( active_jobs.empty() )
        terminated_all_threads.notify_all();
}

void MainThread::erase( Job& job ) {
    boost::mutex::scoped_lock lock( mutex );
    --job_count;
    if ( job_count == 0 )
        main_thread_wakeup.notify_all();
}

void MainThread::connect_stdio( const job::Config& config ) {
    boost::mutex::scoped_lock lock( mutex );
    io->set_input_stream( &std::cin );
    io->set_output_stream( &std::cout );
    io->set_config( config );
    ++job_count;
    input_acquirer = boost::thread( &MainThread::read_input, this );
}

void MainThread::terminate_running_jobs() {
    assert( input_read_lock.owns_lock() );
    DEBUG("Terminate remaining cars");
    std::for_each( active_jobs.begin(), active_jobs.end(),
                   std::mem_fun(&Job::stop) );
    while ( ! active_jobs.empty() )
        terminated_all_threads.wait( input_read_lock );
}

void MainThread::read_input() {
    std::cout << "# rapidSTORM waiting for commands" << std::endl;
    while ( ! io->received_quit_command() ) {
        int peek = std::cin.peek();
        if ( isspace( peek ) ) {
            std::cin.get();
            continue;
        } else if ( peek == std::char_traits<char>::eof() ) {
            break;
        }
        input_read_lock.lock();
        try {
            io->processCommand( std::cin );
        } catch (const std::bad_alloc& e) {
            std::cerr << "Could not perform action: "
                      << "Out of memory" << std::endl;
        } catch (const std::runtime_error& e) {
            std::cerr << "Could not perform action: "
                      << e.what() << std::endl;
        }
        input_read_lock.unlock();
    }
    boost::mutex::scoped_lock lock( mutex );
    --job_count;
    if ( job_count == 0 ) main_thread_wakeup.notify_all();
}

}
