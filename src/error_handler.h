#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <memory>
#include <stdexcept>
#ifdef EXCEPTIONS_AFTER_LONGJMP
#include <setjmp.h>
#endif

/** Exception indicating that a signal was received
 *  due to a programming error. Examples are signals
 *  such as SIGSEGV or SIGFPE.
 */
struct invalid_access : public std::logic_error
{
    invalid_access(int signum);
};

/** Exception indicating that process termination was
 *  requested by signal. Examples are signals
 *  such as SIGTERM or SIGINT. */
struct termination_signal : public std::runtime_error
{
    termination_signal(int signum);
};

struct uncaught_exception : public std::logic_error
{
    uncaught_exception(const std::exception&);
    uncaught_exception();
};

class SignalHandler {
    struct Pimpl;
    std::auto_ptr<Pimpl> pimpl;

  public:
    SignalHandler();
    ~SignalHandler();

    static void initialize();
#ifdef EXCEPTIONS_AFTER_LONGJMP
    jmp_buf& get_jump_buffer();
    void handle_error( int jump_result );
#endif
};

#ifdef EXCEPTIONS_AFTER_LONGJMP
/** The SIGNAL_HANDLER_PANIC_POINT defines the panic point for a signal handler. 
 * A signal handler works only as long as its
 * panic point is defined and in scope. */
#define SIGNAL_HANDLER_PANIC_POINT(signal_handler_reference) \
    signal_handler_reference.handle_error( \
        setjmp( \
            (signal_handler_reference).get_jump_buffer() ) )

#else
#define SIGNAL_HANDLER_PANIC_POINT(x)
#endif

#endif
