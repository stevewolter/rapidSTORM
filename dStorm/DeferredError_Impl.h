#ifndef DEFERRED_ERROR_IMPL_H
#define DEFERRED_ERROR_IMPL_H

#include "DeferredError.h"

namespace dStorm {

template <typename Arg>
bool DeferredErrorBuffer::insert( Arg argument )
{
    Thread * t = Thread::current_thread();
    for (int i = begin; i < end; i++) {
        int s = i % size;
        if ( buffer[s].handles( t ) ) {
            return false;
        }
    }

    int fresh = end++;
    if ( (fresh - begin) >= size ) just_die();
    new(buffer+fresh) DeferredError( t, argument );
    return true;
}

bool DeferredErrorBuffer::empty() {
    return current >= end;
}

}

#endif
