#ifndef DEFERRED_ERROR_H
#define DEFERRED_ERROR_H

#include <dStorm/helpers/thread.h>
#include "helpers/MayBeASignal.h"

namespace dStorm {

class DeferredError {
    Thread *thread;
    int signal;
    bool terminate_program, do_cancel, finished;
    std::auto_ptr<std::string> message;

  public:
    /** Signal-handler-safe constructor. */
    DeferredError( Thread* thread, int signal_number );
    DeferredError( Thread* object, std::string message );

    bool handles( Thread *object ) { return object == thread; }
    void make_error_message();
    MayBeASignal handle_error();
    void notice_dead_thread();
    bool thread_died() { return finished; }
};

class DeferredErrorBuffer
{
    ost::Mutex begin_mutex;
    DeferredError *buffer;
    int begin, current, end, size;
    void just_die();

  public:
    DeferredErrorBuffer(int size);
    ~DeferredErrorBuffer();

    /**@returns Whether an insertion took place. */
    template <typename Arg>
    bool insert( Arg argument );

    DeferredError *find_matching( Thread* thread );

    inline bool empty();
    MayBeASignal handle_first_error();
    void clean_up_for_thread( Thread *thread );
};

}

#endif
