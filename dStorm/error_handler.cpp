#define ERROR_HANDLER_CPP
#include "debug.h"

#include "error_handler.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif
#ifdef HAVE_PROCESS_H
#include <process.h>
#endif

#include <exception>
#include <dStorm/helpers/thread.h>
#include <signal.h>
#include <stdlib.h>
#include <semaphore.h>
#include <dStorm/helpers/errors.h>
#include "DeferredError.h"
#include "DeferredError_Impl.h"
#include <string.h>
#include <errno.h>
#include <dStorm/helpers/BlockingThreadRegistry.h>

#include <stdio.h>
#include <set>

namespace dStorm {

bool ErrorHandler::global_termination_flag = false;

struct ErrorHandler::Pimpl
: public ost::Runnable
{
    ost::Mutex mutex;
    ost::Condition cleanups_empty;
    ErrorHandler& handler_ref;
    std::list<Cleanup*> cleanups;
    std::set<dStorm::Runnable*> emergency_callbacks;
    std::string program, panic_arg;
    char **emergency_call;

    sem_t semaphore;
    dStorm::DeferredErrorBuffer errors_buffer;

    static void cleanup_dead_thread(dStorm::helpers::ThreadStage,
                                    void *, dStorm::Thread *);

    void terminate_with_exit(const std::exception& reason);

    static Pimpl *current_handler;
    Pimpl *last_current_handler;

    Pimpl(ErrorHandler& p,
          const char *program_path, const char *panic_mode_call);
    ~Pimpl();

    static void set_catchers(bool reset = false);
    void run() { set_catchers(); }
    static void on_termination_signal(int signum);
    static void on_unrecoverable_signal(int signum);
    static void on_thread_cancelling_signal(int);
    static void on_terminate();

    void rebuild_emergency_call();
    void destruct_emergency_call();
};

struct ErrorHandler::Cleanup::Pimpl {
    ErrorHandler::Pimpl *impl;
    std::list<std::string> args;
    std::list<Cleanup*>::iterator to_del;
};

ErrorHandler::Pimpl *
ErrorHandler::Pimpl::current_handler
    = NULL;

ErrorHandler::Pimpl::Pimpl(ErrorHandler& p, const char *pp, const char *pm)
: cleanups_empty(mutex),
  handler_ref(p), emergency_call(NULL), errors_buffer(32)
{
    program = pp;
    panic_arg = pm;
    rebuild_emergency_call();

    sem_init( &semaphore, 0, 0 );
    dStorm::helpers::set_semaphore_for_report_of_dead_threads( &semaphore );
    dStorm::helpers::set_error_cleanup_for_threads(cleanup_dead_thread, this);
    if ( current_handler == NULL ) {
        set_catchers();
#ifdef PTW32_VERSION
        DEBUG("Setting global thread initializer");
        dStorm::Thread::set_thread_initializer( *this );
#endif
    }
    last_current_handler = current_handler;
    current_handler = this;
    DEBUG("Set error handler to " << this);
}

void ErrorHandler::Pimpl::terminate_with_exit
        (const std::exception& reason) {
    std::cerr << reason.what() << "\n";
    std::cerr << "Terminating program since I have no other recovery method available on this platform and compiler. Sorry."
              << std::endl;
    exit(1);
}


ErrorHandler::Pimpl::~Pimpl() 
{
    current_handler =
        last_current_handler;
    if ( current_handler != NULL ) {
        dStorm::helpers::set_semaphore_for_report_of_dead_threads( &current_handler->semaphore );
        dStorm::helpers::set_error_cleanup_for_threads(cleanup_dead_thread, current_handler);
        ost::MutexLock lock(mutex);
        ost::MutexLock lock2(current_handler->mutex);
        current_handler->cleanups.splice( current_handler->cleanups.end(),
                                          cleanups );
        current_handler->rebuild_emergency_call();
    } else {
        set_catchers(true);
    }
    destruct_emergency_call();
}

void ErrorHandler::Pimpl::cleanup_dead_thread
( dStorm::helpers::ThreadStage s, void *me, dStorm::Thread *t) 
{
    Pimpl &m = *(Pimpl*)me;

    if ( s == dStorm::helpers::BeforeDestruction ) {
        dStorm::DeferredError* e = m.errors_buffer.find_matching( t );
        if ( e ) e->make_error_message();
    } else
        m.errors_buffer.clean_up_for_thread( t );
}

void ErrorHandler::Pimpl::on_termination_signal(int n) 
{
    if ( current_handler->errors_buffer.insert( n ) )
        sem_post( &current_handler->semaphore );
}

void ErrorHandler::Pimpl::on_thread_cancelling_signal(int n) 
{
    DEBUG("Thread-cancelling signal " << n);
    if ( current_handler->errors_buffer.insert( n ) ) {
        DEBUG("Posting semaphore");
        sem_post( &current_handler->semaphore );
    }
    while ( true ) 
#ifndef PTW32_VERSION
        pause();
#else
        Sleep(100000);
#endif
}

void ErrorHandler::Pimpl::on_unrecoverable_signal(int n) 
{
    std::cerr << "Got unrecoverable signal " << n << ". "
              << "Trying to launch emergency handler." << std::endl; 
    /** OK, we have prepared the emergency call for just this event.
     *  Fire up the execv and let another process clean our mess. */
    current_handler->emergency_call[2][1] += n%10;
    current_handler->emergency_call[2][0] += (n%100)/10;
#ifdef SIGPROF
    signal(SIGPROF, SIG_IGN);
#endif
    execvp( current_handler->emergency_call[0], 
           current_handler->emergency_call );
}

void ErrorHandler::Pimpl::on_terminate() {
    static bool recursive = false, caught_recursive = false;

    DEBUG("Unhandled or unexpected exception");
    std::string message;
    if ( !recursive ) {
        try {
            DEBUG("Re-throwing unhandled exception");
            recursive = true;
            throw;
            recursive = false;
            if ( caught_recursive ) {
                caught_recursive = false; return;
            }
        } catch (const std::exception& e) {
            message = "Uncaught exception: " + std::string(e.what());
        } catch (...) {
            message = "Uncaught exception of unknown type.";
        }
        DEBUG("Re-thrown unhandled exception");
    } else {
        message = "std::terminate was called without an active exception.";
    }

    if ( current_handler->errors_buffer.insert( message ) )
        sem_post( &current_handler->semaphore );
}

void ErrorHandler::Pimpl::set_catchers(bool reset) {
    void (*term_handler)(int num) = on_termination_signal;
    void (*unrecov_handler)(int num) = 
#ifdef EXECVP_ON_CRITICAL_SIGNAL
        on_thread_cancelling_signal;
#else
        on_unrecoverable_signal;
#endif
    if ( getenv("RAPIDSTORM_DISABLE_SIGNAL_HANDLER") ) {
        std::cerr << ("Signal handling disabled by environment variable") << std::endl;
        term_handler = SIG_DFL;
        unrecov_handler = SIG_DFL;
    } else if ( reset ) {
        term_handler = SIG_DFL;
        unrecov_handler = SIG_DFL;
    } else {
        std::set_terminate( on_terminate );
        std::set_unexpected( on_terminate );
    }

    DEBUG("Installing signal handlers");
#ifdef SIGHUP
    signal( SIGHUP, term_handler );
#endif
#ifdef SIGINT
    signal( SIGINT, term_handler );
#endif
#ifdef SIGTERM
    signal( SIGTERM, term_handler );
#endif
#ifdef SIGQUIT
    signal( SIGQUIT, term_handler );
#endif
#ifdef SIGILL
    signal( SIGILL, unrecov_handler );
#endif
#ifdef SIGABRT
    signal( SIGABRT, term_handler );
#endif
#ifdef SIGFPE
    signal( SIGFPE, unrecov_handler );
#endif
#ifdef SIGSEGV
    signal( SIGSEGV, unrecov_handler );
#endif
#ifdef SIGPIPE
    signal( SIGPIPE, term_handler );
#endif
#ifdef SIGALRM
    signal( SIGALRM, term_handler );
#endif

}

ErrorHandler::ErrorHandler(const char *pp, const char *pm) 
: pimpl( new Pimpl(*this, pp,pm) ),
  blocking_thread_registry( new BlockingThreadRegistry() )
{
}

ErrorHandler::~ErrorHandler() 
{}

MayBeASignal
ErrorHandler::handle_errors_until_all_detached_threads_quit() {
    std::auto_ptr<MayBeASignal> last_error;
    while ( true ) {
        sem_wait( &pimpl->semaphore );

        /* Some event happened. If it was an error, then the error
         * count will have increased. */
        if ( ! pimpl->errors_buffer.empty() ) {
            DEBUG("Handling error");
            DeferredError* e = pimpl->errors_buffer.get_first_error();
            global_termination_flag = true;
            /* Error might be stale by now, indicated by e == NULL.
             * Assume the worst. */
            if ( e && ! e->should_terminate() ) {
                std::cerr << e->error_message() << std::endl;
            } else if ( e && e->can_terminate_normally() ) {
                std::cerr << e->error_message() << " Terminating." 
                          << std::endl;
                /* Sleep a little to give processes the chance to react. */
    #ifdef HAVE_USLEEP
                usleep(100 * 1000);
    #elif HAVE_WINDOWS_H
                Sleep(100);
    #endif
                DEBUG("Cancelling blocking threads");
                blocking_thread_registry->cancel_blocking_threads();
                if ( e->was_caused_by_signal() )
                    last_error.reset( new MayBeASignal(e->signal_number()) );
                else
                    last_error.reset( new MayBeASignal(e->should_terminate() ) );
            } else {
                std::cerr << e->error_message() << " "
                             "Running emergency handler and terminating." 
                          << std::endl;
                std::set<dStorm::Runnable*>& set 
                    = pimpl->emergency_callbacks;
                std::set<dStorm::Runnable*>::iterator i;
                for ( i = set.begin(); i != set.end(); i++ )
                    (*i)->run();

                ost::MutexLock lock( pimpl->mutex );
                while ( ! pimpl->cleanups.empty() )
                    pimpl->cleanups_empty.wait();

                if ( e && e->was_caused_by_signal() ) {
#ifndef PTW32_VERSION
                    signal( e->signal_number(), SIG_DFL );
                    kill( getpid(), e->signal_number() );
#else
                    exit(e->signal_number());
#endif
                } else  
                    exit(1);
            }
            DEBUG("Handled error");
        } else {
            /* The error count hasn't increased, so the post has been 
             * induced by the termination of the last detached thread. */
            DEBUG("Detached all threads");
            return (last_error.get()) ? *last_error : MayBeASignal(false);
        }
    }
}

void ErrorHandler::Pimpl::rebuild_emergency_call() {
    destruct_emergency_call();

    std::list<std::string> args;
    for (std::list<Cleanup*>::const_iterator i = cleanups.begin();
                                             i != cleanups.end(); i++)
    {
        const CleanupArgs& l = (*i)->pimpl->args;
        for ( CleanupArgs::const_iterator j = l.begin(); j != l.end(); j++)
            args.push_back( *j );
    }
    
    emergency_call = new char*[ args.size() + 4 ];
    emergency_call[0] = strdup(program.c_str());
    emergency_call[1] = strdup(panic_arg.c_str());
    emergency_call[2] = strdup("00");
    int current = 3;
    for (std::list<std::string>::iterator i = args.begin(); 
                                          i != args.end(); i++)
    {
        emergency_call[current++] = strdup(i->c_str());
    }
    emergency_call[current] = NULL;
}

void ErrorHandler::Pimpl::destruct_emergency_call() {
    if ( emergency_call != NULL ) {
        for (char **p = emergency_call; *p != NULL; p++)
            free(*p);
        delete[] emergency_call;
        emergency_call = NULL;
    }
}

ErrorHandler::Cleanup::Cleanup( ErrorHandler::Pimpl& impl, const CleanupArgs& args ) 
: pimpl( new Pimpl() )
{
    DEBUG("Creating cleanup " << this);
    ost::MutexLock lock(impl.mutex);
    pimpl->impl = &impl;
    pimpl->args = args;
    pimpl->to_del = impl.cleanups.insert( impl.cleanups.end(), this );
    impl.rebuild_emergency_call();
}

ErrorHandler::Cleanup::~Cleanup() {
    DEBUG("Destroying cleanup " << this);
    ost::MutexLock lock(pimpl->impl->mutex);
    pimpl->impl->cleanups.erase( pimpl->to_del );
    if ( pimpl->impl->cleanups.empty() )
        pimpl->impl->cleanups_empty.signal();
    pimpl->impl->rebuild_emergency_call();
}

ErrorHandler& ErrorHandler::get_current_handler() {
    DEBUG("Returning handler " << Pimpl::current_handler);
    return Pimpl::current_handler->handler_ref;
}

void ErrorHandler::add_emergency_callback(dStorm::Runnable& runnable) {
    ost::MutexLock lock(pimpl->mutex);
    pimpl->emergency_callbacks.insert( &runnable );
}

void ErrorHandler::remove_emergency_callback(dStorm::Runnable& runnable) {
    ost::MutexLock lock(pimpl->mutex);
    pimpl->emergency_callbacks.erase( &runnable );
}


}
