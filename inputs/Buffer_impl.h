#ifndef DSTORM_INPUT_BUFFER_IMPL_H
#define DSTORM_INPUT_BUFFER_IMPL_H

#include <cassert>
#include <dStorm/helpers/thread.h>
#include <limits>

#include "Buffer.h"
#include <dStorm/input/Source.h>

#include "Buffer_impl_iterator.h"
#include <dStorm/input/Source_impl.h>

using namespace std;

namespace dStorm {
namespace input {

template <typename Type, int Dim>
std::ostream &operator<<( std::ostream& o, const dStorm::Image<Type,Dim>& i ) { return o << i.frame_number(); }
std::ostream &operator<<( std::ostream& o, const dStorm::output::LocalizedImage& i ) { return o; }

template<typename Object, bool RunConcurrently>
Buffer<Object,RunConcurrently>::Buffer(std::auto_ptr< Source<Object> > src) 
: AdapterSource<Object>(src),
  fetch_is_finished(false),
  mayDiscard( false ), need_to_init_iterators(false),
  is_transparent(true),
  new_data(mutex),
  next_output( buffer.begin() )
{
    if ( RunConcurrently ) {
        concurrent_fetch = boost::thread( &Buffer<Object,RunConcurrently>::run, this );
    } else {
        need_to_init_iterators = true;
    }

    DEBUG("Created image buffer, concurrent fetch is " << RunConcurrently);
}

template<typename Object, bool RunConcurrently>
Buffer<Object,RunConcurrently>::~Buffer() {
    DEBUG("Joining subthread");
    fetch_is_finished = true;
    concurrent_fetch.join();
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
    this->base().dispatch(m);
    DEBUG("Dispatched message " << m.to_string() << " to buffer");
}

template<typename Object, bool RunConcurrently>
void Buffer<Object,RunConcurrently>::run() 
{
    try {
        typename Source<Object>::iterator i, e;

        DEBUG("Iterating source in subthread");
        for (i = this->base().begin(), e = this->base().end(); i != e; ++i ) 
        {
            ost::MutexLock lock(mutex);
            buffer.push_back( *i );
            if ( next_output == buffer.end() ) --next_output;
            new_data.signal();
            if ( fetch_is_finished ) break;
        }
        DEBUG("Iterated source in subthread, fetch_is_finished " << fetch_is_finished);
    } catch (const std::exception& e) {
        std::cerr << "Error in reading input: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown error in reading input" << std::endl;
    }

    fetch_is_finished = true;
    new_data.broadcast();
}

template<typename Object, bool RunConcurrently>
typename Buffer<Object,RunConcurrently>::Slots::iterator 
Buffer<Object,RunConcurrently>::get_free_slot() 
{
    ost::MutexLock lock(mutex);
    while ( true ) {
        if ( next_output != buffer.end() ) {
            DEBUG("Returning stored object " << *next_output);
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
            DEBUG("Got input " << buffer.back());
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
    if ( ! is_transparent ) {
        assert( ! need_to_init_iterators );
        return typename Source<Object>::iterator( iterator(*this) );
    } else {
        return this->base().begin();
    }
}

template<typename Object, bool RunConcurrently>
typename Source<Object>::iterator
Buffer<Object,RunConcurrently>::end() 
{ 
    if ( ! is_transparent ) {
        assert( ! need_to_init_iterators );
        return typename Source<Object>::iterator( iterator() );
    } else {
        return this->base().end();
    }
}

template<typename Object, bool RunConcurrently>
typename Source<Object>::TraitsPtr
Buffer<Object,RunConcurrently>::get_traits( BaseSource::Wishes w ) 
{
    typename Source<Object>::TraitsPtr t = this->base().get_traits(w);
    BaseSource::Capabilities c = this->base().capabilities();
    BaseSource::Capability providing[] = 
        { BaseSource::Repeatable, BaseSource::ConcurrentIterators };
    is_transparent = true;
    for (int i = 0; i < 2; ++i)
        is_transparent = is_transparent &&
            ( c.test( providing[i] ) || ! w.test( providing[i] ) );
    if ( !is_transparent && need_to_init_iterators ) {
        current_input = this->base().begin();
        end_of_input = this->base().end();
        need_to_init_iterators = false;
    }
    return t;
}

}
}

#endif
