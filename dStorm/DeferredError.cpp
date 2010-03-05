#include "DeferredError.h"
#include <stdlib.h>
#include <signal.h>

namespace dStorm {

static std::string signame(int num);
static bool sig_is_deadly(int num);

DeferredErrorBuffer::DeferredErrorBuffer(int size)
: begin(0), current(0), end(0), size(size)
{
    buffer = (DeferredError*)malloc(sizeof(DeferredError) * size);
}

DeferredErrorBuffer::~DeferredErrorBuffer() {
    for (int i = std::max(begin, end-size); i != end; i++) {
        buffer[i % size].~DeferredError();
    }
    free(buffer);
}

MayBeASignal DeferredErrorBuffer::handle_first_error() 
{
    int i = current++;
    if ( i >= end-size ) {
        return buffer[i].handle_error();
    } else {
        return MayBeASignal(false);
    }
}

void DeferredErrorBuffer::clean_up_for_thread( Thread *thread ) {
    DeferredError* e = find_matching(thread);
    if ( e ) {
        e->notice_dead_thread();
        ost::MutexLock lock( begin_mutex );
        while ( begin < current && buffer[begin%size].thread_died() ) {
            int s = begin++;
            buffer[s%size].~DeferredError();
        }
    }
}

DeferredError::DeferredError( Thread* thread, int signal_number ) 
: thread(thread), signal(signal_number),
  terminate_program( sig_is_deadly(signal_number) ),
  do_cancel( ! terminate_program ),
  finished(false)
{
}

DeferredError::DeferredError( Thread* object, std::string message )
: thread(thread), terminate_program(false), do_cancel( true ),
  finished(false),
  message( new std::string( message ) )
{
}

void DeferredError::make_error_message() {
    if ( message.get() != NULL ) 
        return;
    else if ( sig_is_deadly( signal ) ) {
        message.reset( new std::string(
            "Terminated by " + signame(signal) + " signal." ) );
    } else {
        message.reset( new std::string(
            "An critical error in programming raised the "
            "signal for " + signame(signal) + "." ) );
    }
}

MayBeASignal DeferredError::handle_error() {
    make_error_message();

    if ( terminate_program ) {
        if ( signal != 0 )
            return MayBeASignal(signal);
        else
            return MayBeASignal(true);
    } else if ( do_cancel ) {
        do_cancel = false;
        if ( thread ) {
            thread->abnormal_termination(*message);
            thread->cancel();
            return MayBeASignal(false);
        } else {
            std::cerr << *message << " The faulty computation could not "
                         "be localized, thus the whole program is "
                         "aborted now. Sorry." << std::endl;
            return MayBeASignal(true);
        }
    } else {
        std::cerr << *message << std::endl;
        return MayBeASignal(false);
    }
}

void DeferredError::notice_dead_thread() {
    do_cancel = false;
    finished = true;
}

DeferredError *DeferredErrorBuffer::find_matching( Thread* thread )
{
    for (int i = begin; i < end; i++) {
        if ( buffer[i%size].handles(thread) )
            return &buffer[i%size];
    }
    return NULL;
}

void DeferredErrorBuffer::just_die() 
{
    _Exit(2);
}

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

}
