#define ERROR_HANDLER_CPP
#include "error_handler.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

#include <exception>
#include <dStorm/helpers/thread.h>
#include <signal.h>
#ifdef HANDLE_SIGNALS_ASYNCHRONOUSLY
#include <semaphore.h>
#include <dStorm/helpers/errors.h>
#include "DeferredError.h"
#include "DeferredError_Impl.h"
#endif

#include "debug.h"


struct SignalHandler::Pimpl
: public ost::Runnable
{
#ifndef HANDLE_SIGNALS_ASYNCHRONOUSLY
#ifdef EXCEPTIONS_AFTER_LONGJMP
    jmp_buf panic_point;
    std::auto_ptr<std::exception> reason;
    enum ErrorType { 
        AllGood = 0,
        InvalidAccess = 1,
        TerminationSignal = 2,
        UncaughtException = 3 };

    void handle_longjmp(int rv_of_setjmp);
    void throw_reason(ErrorType r);
    void deliver_to_thread(ErrorType r);
#endif
#else
    sem_t semaphore;
    dStorm::DeferredErrorBuffer errors_buffer;

    static void cleanup_dead_thread(dStorm::helpers::ThreadStage,
                                    void *, dStorm::Thread *);
#endif

    void terminate_with_exit(const std::exception& reason);

    void handle_signal(int signum);

    static Pimpl *current_handler;
    Pimpl *last_current_handler;

    Pimpl();
    ~Pimpl();

    static void set_catchers();
    void run() { set_catchers(); }
    static void on_signal(int signum);
    static void on_terminate();
};

SignalHandler::Pimpl *
SignalHandler::Pimpl::current_handler
    = NULL;

SignalHandler::Pimpl::Pimpl()
#ifdef HANDLE_SIGNALS_ASYNCHRONOUSLY
: errors_buffer(32)
#endif
{
#ifdef HANDLE_SIGNALS_ASYNCHRONOUSLY
    sem_init( &semaphore, 0, 0 );
    dStorm::helpers::set_semaphore_for_report_of_dead_threads( &semaphore );
    dStorm::helpers::set_error_cleanup_for_threads(cleanup_dead_thread, this);
#endif
    if ( current_handler == NULL ) {
        set_catchers();
#ifdef PTW32_VERSION
        DEBUG("Setting global thread initializer");
        dStorm::Thread::set_thread_initializer( *this );
#endif
    }
    last_current_handler = current_handler;
    current_handler = this;
}

void SignalHandler::Pimpl::terminate_with_exit
        (const std::exception& reason) {
    std::cerr << reason.what() << "\n";
    std::cerr << "Terminating program since I have no other recovery method available on this platform and compiler. Sorry."
              << std::endl;
    exit(1);
}


#ifndef HANDLE_SIGNALS_ASYNCHRONOUSLY
#ifdef EXCEPTIONS_AFTER_LONGJMP
void SignalHandler::Pimpl::handle_longjmp(int rv_of_setjmp)
{
    ErrorType r = (ErrorType)rv_of_setjmp;
    if ( r == AllGood ) {
        /* Normal execution. */
    } else {
        DEBUG("At panic point with me " << this);
        if ( reason.get() == NULL )
            throw std::logic_error("Reason for long jump failure not set");

        DEBUG("At panic point with reason " << reason.get() << " " << reason->what());
        throw_reason(r);
    }
}

void SignalHandler::Pimpl::deliver_to_thread( ErrorType r ) 
{
    dStorm::Thread *ct = dStorm::Thread::current_thread();
    if ( ct == NULL ) {
        DEBUG("No delivery thread found, jumping to panic point");
        longjmp( panic_point, r );
    } else {
        DEBUG("Delivering signal to thread " << ct->description());
        ct->abnormal_termination( reason->what() );
    }
}
#endif
#endif

SignalHandler::Pimpl::~Pimpl() 
{
    current_handler =
        last_current_handler;
#ifdef HANDLE_SIGNALS_ASYNCHRONOUSLY
    if ( current_handler != NULL ) {
        dStorm::helpers::set_semaphore_for_report_of_dead_threads( &current_handler->semaphore );
        dStorm::helpers::set_error_cleanup_for_threads(cleanup_dead_thread, current_handler);
    }
#endif
}

void SignalHandler::Pimpl::cleanup_dead_thread
( dStorm::helpers::ThreadStage s, void *me, dStorm::Thread *t) 
{
    Pimpl &m = *(Pimpl*)me;

    if ( s == dStorm::helpers::BeforeDestruction ) {
        dStorm::DeferredError* e = m.errors_buffer.find_matching( t );
        if ( e ) e->make_error_message();
    } else
        m.errors_buffer.clean_up_for_thread( t );
}

#ifndef HANDLE_SIGNALS_ASYNCHRONOUSLY
#ifdef EXCEPTIONS_AFTER_LONGJMP
void SignalHandler::Pimpl::throw_reason(ErrorType r) 
{
    if ( r == InvalidAccess ) {
        DEBUG("Throwing invalid access");
        throw reinterpret_cast<invalid_access&>(*reason);
    } else if ( r == TerminationSignal ) {
        DEBUG("Throwing termination signal");
        throw reinterpret_cast<termination_signal&>(*reason);
    } else if ( r == UncaughtException ) {
        DEBUG("Throwing uncaught exception");
        throw reinterpret_cast<uncaught_exception&>(*reason);
    } else {
        DEBUG("Throwing logic error");
        throw std::logic_error("Unknown reason type in signal handler");
    }
}
#endif
#endif

void SignalHandler::Pimpl::on_signal(int n) 
{
    current_handler->handle_signal(n);
}

void SignalHandler::Pimpl::handle_signal(int n) 
{
#ifdef HANDLE_SIGNALS_ASYNCHRONOUSLY
    if ( errors_buffer.insert( n ) ) {
        sem_post( &semaphore );
        while ( true ) 
#ifdef HAVE_USLEEP
            pause();
#else
            Sleep(1000000);
#endif
        DEBUG("Returned from pause");
    }
#else
#ifndef PTW32_VERSION
    DEBUG("Unmasking signal");
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, n);
    pthread_sigmask(SIG_UNBLOCK, &sigset, NULL);
#endif

    if ( sig_is_deadly(n) ) {
#ifdef EXCEPTIONS_IN_SIGNAL_HANDLERS
        DEBUG("Signal is deadly, throwing exception");
        throw termination_signal(n);
#elif defined(EXCEPTIONS_AFTER_LONGJMP)
        DEBUG("Signal is deadly, jumping to panic point");
        reason.reset( new termination_signal(n) );
        longjmp( panic_point, TerminationSignal );
#else
        terminate_with_exit( termination_signal(n) );
#endif
    } else {
#ifdef EXCEPTIONS_IN_SIGNAL_HANDLERS
        DEBUG("Throwing recoverable exception directly");
        throw invalid_access(n);
#elif defined(EXCEPTIONS_AFTER_LONGJMP)
        DEBUG("Signal is recoverable, delivering to thread");
        reason.reset( new invalid_access(n) );
        deliver_to_thread( InvalidAccess );
#else
        terminate_with_exit( invalid_access(n) );
#endif
    }
#endif
}

void SignalHandler::Pimpl::on_terminate() {
    DEBUG("Unhandled or unexpected exception");
    std::string message;
    try {
        DEBUG("Re-throwing unhandled exception");
        throw;
    } catch (const std::exception& e) {
        message = "Uncaught exception: " + std::string(e.what());
    } catch (...) {
        message = "Uncaught exception of unknown type.";
    }
    DEBUG("Re-thrown unhandled exception");

#ifdef HANDLE_SIGNALS_ASYNCHRONOUSLY
    if ( current_handler->errors_buffer.insert( message ) )
        sem_post( &current_handler->semaphore );
#else
#ifdef EXCEPTIONS_AFTER_LONGJMP
    current_handler->reason = ue;
    current_handler->deliver_to_thread( UncaughtException );
#else
    current_handler->terminate_with_exit(*ue);
#endif
#endif
}

void SignalHandler::Pimpl::set_catchers() {
    if ( getenv("RAPIDSTORM_DISABLE_SIGNAL_HANDLER") ) {
        DEBUG("Signal handling disabled by environment variable");
        return;
    }
    DEBUG("Installing signal handlers");
#ifdef SIGHUP
    signal( SIGHUP, on_signal );
#endif
#ifdef SIGINT
    //signal( SIGINT, on_signal );
#endif
#ifdef SIGTERM
    signal( SIGTERM, on_signal );
#endif
#ifdef SIGQUIT
    signal( SIGQUIT, on_signal );
#endif
#ifdef SIGILL
    signal( SIGILL, on_signal );
#endif
#ifdef SIGABRT
    signal( SIGABRT, on_signal );
#endif
#ifdef SIGFPE
    signal( SIGFPE, on_signal );
#endif
#ifdef SIGSEGV
    signal( SIGSEGV, on_signal );
#endif
#ifdef SIGPIPE
    signal( SIGPIPE, on_signal );
#endif
#ifdef SIGALRM
    signal( SIGALRM, on_signal );
#endif

    //std::set_terminate( on_terminate );
    //std::set_unexpected( on_terminate );
}

SignalHandler::SignalHandler() 
: pimpl( new Pimpl() )
{
}

SignalHandler::~SignalHandler() 
{}

#ifndef HANDLE_SIGNALS_ASYNCHRONOUSLY
#ifdef EXCEPTIONS_AFTER_LONGJMP
jmp_buf& SignalHandler::get_jump_buffer() {
    return pimpl->panic_point;
}

void SignalHandler::handle_error(int r) {
    if ( r != 0 )
        pimpl->handle_longjmp(r);
}
#endif
#endif

void SignalHandler::handle_errors_until_all_detached_threads_quit() {
#ifdef HANDLE_SIGNALS_ASYNCHRONOUSLY
    while ( true ) {
        sem_wait( &pimpl->semaphore );

        /* Some event happened. If it was an error, then the error
         * count will have increased. */
        if ( ! pimpl->errors_buffer.empty() ) {
            DEBUG("Handling error");
            pimpl->errors_buffer.handle_first_error();
            DEBUG("Handled error");
        } else {
            /* The error count hasn't increased, so the post has been 
             * induced by the termination of the last detached thread. */
            DEBUG("Detached all threads");
            break;
        }
    }
#else
    Thread::wait_for_detached_threads();
#endif
}
