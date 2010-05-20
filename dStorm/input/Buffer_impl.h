#ifndef DSTORM_INPUT_BUFFER_IMPL_H
#define DSTORM_INPUT_BUFFER_IMPL_H

#include <cassert>
#include <dStorm/helpers/thread.h>
#include <limits>

#include "Buffer.h"
#include "Source.h"
#include "Slot.h"

#include "Buffer_impl_iterator.h"

#include "debug.h"

using namespace std;

namespace dStorm {
namespace input {

template<typename Object>
Buffer<Object>::Buffer(std::auto_ptr< Source<Object> > src) 
: Source<Object>( src->getNode(), BaseSource::Flags().set(BaseSource::Repeatable) ),
  ost::Thread("Input fetcher"),
  mayDiscard( false ), concurrent_fetch( src->flags.test(BaseSource::TimeCritical) ),
  new_data(mutex)
{
    this->source = src;
    if ( concurrent_fetch ) {
        ost::MutexLock lock(mutex);
        run();
        /* Wait until the source has initialized. */
        new_data.wait();
    } else {
        current_input = source->begin();
        end_of_input = source->end();
    }

    DEBUG("Created image buffer");
}

template<typename Object>
Buffer<Object>::~Buffer() {
    DEBUG("Destructing " << (void*)this);
    source.reset( NULL );
    DEBUG("Destructed source for " << this);
}

template<typename Object>
void Buffer<Object>::dispatch(BaseSource::Messages m) {
    if ( m.test( BaseSource::MightNeedRepeat ) ) {
        m.reset( BaseSource::MightNeedRepeat );
        mayDiscard = true;
    }
    if ( m.test( BaseSource::WillNeverRepeatAgain ) ) {
        m.reset( BaseSource::WillNeverRepeatAgain );
        ost::MutexLock lock(mutex);
        if ( !mayDiscard ) {
            mayDiscard = true;
            processed.clear();
        }
    }
    if ( m.test( BaseSource::RepeatInput ) ) {
        m.reset( BaseSource::RepeatInput );
        ost::MutexLock lock(mutex);
        unprocessed.splice( unprocessed.begin(), processed );
    }
    source->dispatch(m);
}

template<typename Object>
void Buffer<Object>::makeAllUntouched() {
}

template<typename Object>
void Buffer<Object>::run() 
{
    try {
        current_input = source->begin(); 
        end_of_input = source->end();
        new_data.signal();

        for ( ; current_input != end_of_input; ++current_input ) 
        {
            ost::MutexLock lock(mutex);
            unprocessed.push_back( *current_input );
            new_data.signal();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in reading input: " << e.what() << std::endl;
        current_input = end_of_input;
        new_data.signal();
    }
}

template<typename Object>
simparm::Node& Buffer<Object>::getConfig()
{
    return source->getNode();
}

template<typename Object>
typename Buffer<Object>::Slots::iterator Buffer<Object>::get_free_slot() 
{
    ost::MutexLock lock(mutex);
    while ( true ) {
        if ( ! unprocessed.empty() ) {
            current.splice( current.begin(), unprocessed, unprocessed.begin() );
            new_data.signal();
            return current.begin();
        } else if ( current_input == end_of_input ) {
            return current.end();
        } else if ( ! concurrent_fetch ) {
            current.push_front( *current_input );
            ++current_input;
            new_data.signal();
            return current.begin();
        } else {
            new_data.wait();
        }
    }
}

template<typename Object>
void Buffer<Object>::discard( typename Slots::iterator slot ) {
    ost::MutexLock lock(mutex);
    if ( mayDiscard ) {
        current.erase( slot );
    } else if ( slot != current.end() ) {
        typename Slots::iterator insert_pos = processed.end(); 
        for ( typename Slots::reverse_iterator 
               i = processed.rbegin(); i != processed.rend(); i++ ) 
            if ( *slot < *i ) {
                insert_pos = --i.base();
            } else {
                break;
            }

        processed.splice( insert_pos, current, slot );
    }
}

template<typename Object>
typename Source<Object>::iterator
Buffer<Object>::begin() 
{ 
    return typename Source<Object>::iterator( iterator().attach(*this) );
}

template<typename Object>
typename Source<Object>::iterator
Buffer<Object>::end() 
{ 
    return typename Source<Object>::iterator();
}

template<typename Object>
typename Source<Object>::TraitsPtr
Buffer<Object>::get_traits() {
    return source->get_traits();
}

}
}

#endif
