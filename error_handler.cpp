#define ERROR_HANDLER_CPP
#include "error_handler.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <exception>
#include <dStorm/helpers/thread.h>
#include <signal.h>

#include "debug.h"

static std::string signame(int num)
{
    if ( false )
        return "unknown";
#ifdef SIGHUP
    else if ( num == SIGHUP )
        return "terminal hang-up";
#endif
#ifdef SIGINT
    else if ( num == SIGINT )
        return "interrupt from terminal";
#endif
#ifdef SIGQUIT
    else if ( num == SIGQUIT )
        return "quit from terminal";
#endif
#ifdef SIGTERM
    else if ( num == SIGTERM )
        return "termination";
#endif
#ifdef SIGSEGV
    else if ( num == SIGSEGV )
        return "segmentation fault";
#endif
#ifdef SIGPIPE
    else if ( num == SIGPIPE )
        return "closed pipe";
#endif
#ifdef SIGALRM
    else if ( num == SIGALRM )
        return "alarm";
#endif
#ifdef SIGFPE
    else if ( num == SIGFPE )
        return "floating-point error";
#endif
#ifdef SIGABRT
    else if ( num == SIGABRT )
        return "abortion";
#endif
    else 
        return "unknown";
}

invalid_access::invalid_access(int num)
: std::logic_error(
    "A programming error caused the signal "
    "for " + signame(num) + " to be raised." )
{
}

termination_signal::termination_signal(int num)
: std::runtime_error(
    "Received signal for "
        + signame(num) + "." )
{
}

uncaught_exception::uncaught_exception(const std::exception& e)
: std::logic_error(
    std::string("Uncaught exception: ")
        + e.what())
{}

uncaught_exception::uncaught_exception()
: std::logic_error("Uncaught exception of unknown type")
{}

static bool sig_is_deadly(int num) {
    return ( false 
#ifdef SIGHUP
    || num == SIGHUP
#endif
#ifdef SIGINT
    || num == SIGINT
#endif
#ifdef SIGQUIT
    || num == SIGQUIT
#endif
#ifdef SIGTERM
    || num == SIGTERM 
#endif
    );
}

struct SignalHandler::Pimpl
: public ost::Runnable
{
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

SignalHandler::Pimpl::Pimpl() {
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

SignalHandler::Pimpl::~Pimpl() 
{
    current_handler =
        last_current_handler;
}

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

void SignalHandler::Pimpl::on_signal(int n) 
{
    current_handler->handle_signal(n);
}

void SignalHandler::Pimpl::handle_signal(int n) 
{
    DEBUG("Got signal " << n);
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
}

void SignalHandler::Pimpl::on_terminate() {
    DEBUG("Unhandled or unexpected exception");
    std::auto_ptr<uncaught_exception> ue;
    try {
        DEBUG("Jumping to panic point after uncaught "
              "exception");
        throw;
    } catch (const std::exception& e) {
        ue.reset( new uncaught_exception(e) );
    } catch (...) {
        ue.reset( new uncaught_exception() );
    }

#ifdef EXCEPTIONS_AFTER_LONGJMP
    current_handler->reason = ue;
    current_handler->deliver_to_thread( UncaughtException );
#else
    current_handler->terminate_with_exit(*ue);
#endif
}

void SignalHandler::Pimpl::set_catchers() {
    DEBUG("Installing signal handlers");
#ifdef SIGHUP
    signal( SIGHUP, on_signal );
#endif
#ifdef SIGINT
    signal( SIGINT, on_signal );
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
    DEBUG("Installing SIGSEGV handler");
    signal( SIGSEGV, on_signal );
#endif
#ifdef SIGPIPE
    signal( SIGPIPE, on_signal );
#endif
#ifdef SIGALRM
    signal( SIGALRM, on_signal );
#endif

    std::set_terminate( on_terminate );
    std::set_unexpected( on_terminate );
}

SignalHandler::SignalHandler() 
: pimpl( new Pimpl() )
{
}

SignalHandler::~SignalHandler() 
{}

void SignalHandler::initialize() {
    Pimpl::set_catchers();
}

#ifdef EXCEPTIONS_AFTER_LONGJMP
jmp_buf& SignalHandler::get_jump_buffer() {
    return pimpl->panic_point;
}

void SignalHandler::handle_error(int r) {
    if ( r != 0 )
        pimpl->handle_longjmp(r);
}
#endif
