#ifndef DSTORM_INPUT_BUFFER_IMPL_H
#define DSTORM_INPUT_BUFFER_IMPL_H

#include <cassert>
#include <dStorm/helpers/thread.h>
#include <limits>

#include "Buffer.h"
#include "Source.h"
#include "Slot.h"

#include "Buffer_impl_iterator.h"

using namespace std;

namespace dStorm {
namespace input {

template<typename Object, bool RunConcurrently>
Buffer<Object,RunConcurrently>::Buffer(std::auto_ptr< Source<Object> > src) 
: Source<Object>( src->getNode(), BaseSource::Flags().set(BaseSource::Repeatable) ),
  ost::Thread("Input fetcher"),
  mayDiscard( false ),
  new_data(mutex),
  next_output( buffer.begin() ),
  fetch_is_finished(false)
{
    this->source = src;
    if ( RunConcurrently ) {
        run();
    } else {
        current_input = source->begin();
        end_of_input = source->end();
    }

    DEBUG("Created image buffer");
}

template<typename Object, bool RunConcurrently>
Buffer<Object,RunConcurrently>::~Buffer() {
    DEBUG("Destructing " << (void*)this);
    source.reset( NULL );
    DEBUG("Destructed source for " << this);
}

template<typename Object, bool RunConcurrently>
void Buffer<Object,RunConcurrently>::dispatch(BaseSource::Messages m) {
    DEBUG("Dispatching message " << m.to_string() << " to buffer");
    if ( m.test( BaseSource::WillNeverRepeatAgain ) ) {
        m.reset( BaseSource::WillNeverRepeatAgain );
        ost::MutexLock lock(mutex);
        if ( !mayDiscard ) {
            mayDiscard = true;
            buffer.erase( buffer.begin(), next_output );
        }
    }
    if ( m.test( BaseSource::RepeatInput ) ) {
        m.reset( BaseSource::RepeatInput );
        ost::MutexLock lock(mutex);
        if ( mayDiscard ) throw std::runtime_error("Buffer is not repeatable any more");
        next_output = buffer.begin();
    }
    source->dispatch(m);
    DEBUG("Dispatched message " << m.to_string() << " to buffer");
}

template<typename Object, bool RunConcurrently>
void Buffer<Object,RunConcurrently>::run() 
{
    try {
        typename Source<Object>::iterator i, e;

        for (i = source->begin(), e = source->end(); i != e; ++i ) 
        {
            ost::MutexLock lock(mutex);
            std::cerr << "Pushing " << i->frame_number().value() << std::endl;
            buffer.push_back( *i );
            if ( next_output == buffer.end() ) --next_output;
            new_data.signal();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in reading input: " << e.what() << std::endl;
    }

    fetch_is_finished = true;
    new_data.broadcast();
}

template<typename Object, bool RunConcurrently>
simparm::Node& Buffer<Object,RunConcurrently>::getConfig()
{
    return source->getNode();
}

template<typename Object, bool RunConcurrently>
typename Buffer<Object,RunConcurrently>::Slots::iterator 
Buffer<Object,RunConcurrently>::get_free_slot() 
{
    ost::MutexLock lock(mutex);
    while ( true ) {
        if ( next_output != buffer.end() ) {
            DEBUG("Returning stored object");
            return next_output++;
        } else if ( 
             (RunConcurrently) 
                ? fetch_is_finished 
                : current_input == end_of_input )
        {
            DEBUG("Returning empty list");
            return buffer.end();
        } else if ( RunConcurrently ) {
            DEBUG("Waiting for input");
            new_data.wait();
        } else {
            DEBUG("Getting input");
            buffer.push_back( *current_input );
            ++current_input;
            return --buffer.end();
        }
    }
}

template<typename Object, bool RunConcurrently>
void Buffer<Object,RunConcurrently>::discard( typename Slots::iterator slot ) {
    ost::MutexLock lock(mutex);
    if ( mayDiscard && slot != buffer.end() ) {
        buffer.erase( slot );
    } 
}

template<typename Object, bool RunConcurrently>
typename Source<Object>::iterator
Buffer<Object,RunConcurrently>::begin() 
{ 
    return typename Source<Object>::iterator( iterator(*this) );
}

template<typename Object, bool RunConcurrently>
typename Source<Object>::iterator
Buffer<Object,RunConcurrently>::end() 
{ 
    return typename Source<Object>::iterator( iterator() );
}

template<typename Object, bool RunConcurrently>
typename Source<Object>::TraitsPtr
Buffer<Object,RunConcurrently>::get_traits() {
    DEBUG("Getting traits from buffer");
    return source->get_traits();
}

}
}

#endif
