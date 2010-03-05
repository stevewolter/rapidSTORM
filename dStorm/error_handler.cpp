#define ERROR_HANDLER_CPP
#include "debug.h"

#include "error_handler.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
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

namespace dStorm {

bool ErrorHandler::global_termination_flag = false;

struct ErrorHandler::Pimpl
: public ost::Runnable
{
    ErrorHandler& handler_ref;
    std::list<Cleanup*> cleanups;
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
: handler_ref(p), emergency_call(NULL), errors_buffer(32)
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

void ErrorHandler::Pimpl::on_unrecoverable_signal(int n) 
{
    /** OK, we have prepared the emergency call for just this event.
     *  Fire up the execv and let another process clean our mess. */
    current_handler->emergency_call[2][1] += n%10;
    current_handler->emergency_call[2][0] += (n%100)/10;
#ifdef SIGPROF
    signal(SIGPROF, SIG_IGN);
#endif
    execvp( current_handler->emergency_call[0], 
           current_handler->emergency_call );
    /* Error must have occured if code continues. */
    std::cerr << "Error in executing emergency cleanup code: "
              << strerror(errno) << std::endl;
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
    void (*unrecov_handler)(int num) = on_unrecoverable_signal;
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
            MayBeASignal rv = pimpl->errors_buffer.handle_first_error();
            if ( rv.should_terminate() ) {
                global_termination_flag = true;
                /* Sleep a little to give processes the chance to react. */
#ifdef HAVE_USLEEP
                usleep(100 * 1000);
#elif HAVE_WINDOWS_H
                Sleep(100);
#endif
                DEBUG("Cancelling blocking threads");
                blocking_thread_registry->cancel_blocking_threads();
            }
            last_error.reset( new MayBeASignal(rv) );
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
    pimpl->impl = &impl;
    pimpl->args = args;
    pimpl->to_del = impl.cleanups.insert( impl.cleanups.end(), this );
    impl.rebuild_emergency_call();
}

ErrorHandler::Cleanup::~Cleanup() {
    pimpl->impl->cleanups.erase( pimpl->to_del );
    pimpl->impl->rebuild_emergency_call();
}

ErrorHandler& ErrorHandler::get_current_handler() {
    DEBUG("Returning handler " << Pimpl::current_handler);
    return Pimpl::current_handler->handler_ref;
}



}
