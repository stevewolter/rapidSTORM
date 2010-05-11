#ifndef DSTORM_INPUT_SLOT_IMPL_H
#define DSTORM_INPUT_SLOT_IMPL_H
#include "Slot.h"

#include <iostream>
#include <memory>

#include "debug.h"

namespace dStorm {
namespace input {

template <typename Object>
ostream& operator<<(ostream& o, typename Slot<Object>::State s) {
         if ( s == Slot<Object>::Untouched ) o << "untouched";
    else if ( s == Slot<Object>::Delivered ) o << "delivered";
    else if ( s == Slot<Object>::Processed ) o << "processed";
    else if ( s == Slot<Object>::Discarded ) o << "discarded";
    return o;
}

template <typename Object>
Slot<Object>::Slot(Source<Object> &src,int index, const bool& discardable) 

: source(&src), my_index(index), mayDiscard(discardable),
  state(Untouched), dataMutex(), gotData(dataMutex)
{ 
    DEBUG("Initial discarding license: " << discardable);
    if ( ! source->pushes() && ! source->canBePulled() )
        throw std::logic_error
            ("Supplied source can neither push nor pull.");
    if ( src.manages_returned_objects() )
        throw std::logic_error
            ("Supplied source manages objects by itself. To save objects, "
             "dynamically allocated storage is needed.");
}

template <typename Object>
Slot<Object>::Slot(const Slot<Object> &c)
: source(c.source), my_index(c.my_index),
  claims(0), mayDiscard(c.mayDiscard),
  state(Untouched), dataMutex(), gotData(dataMutex)
{}

template <typename Object>
Slot<Object>& Slot<Object>::operator=(const Slot<Object> &c) 
{
  source=(c.source); 
  my_index=(c.my_index),
  claims=(0); 
  state=(Untouched);
  return *this;
}

template <typename Object>
Slot<Object>::~Slot() {
}

template <typename Object>
void Slot<Object>::finish() {
    ost::MutexLock lock(dataMutex);
    if ( --claims > 0 ) return;
    DEBUG("Finished image " << my_index << " at state " 
          << state << " and may " 
             << ((mayDiscard) ? "" : "not ") << "discard");
    if ( mayDiscard ) {
        state = Discarded;
        data.reset(NULL);
        DEBUG("Discarded image " << my_index);
    } else
        state = Processed;
    DEBUG("State after processing is " << state);
}

template <typename Object>
void Slot<Object>::clean() {
    ost::MutexLock lock(dataMutex);

    if ( state == Processed )
        state = Delivered;
    DEBUG("Cleaned state to " << state);
}

template <typename Object>
bool Slot<Object>::discard() {
    ost::MutexLock lock(dataMutex);
    bool hasDiscarded = (data.get() != NULL) && (state == Processed);
    if (state == Processed) {
        DEBUG("Discarding " << my_index);
        data.reset( NULL ); state = Discarded; 
    }
    return hasDiscarded;
}

template <typename Object>
void Slot<Object>::fetchData() {
    while ( state == Untouched || state == Discarded ) {
        if ( source->canBePulled() ) {
            dataMutex.leaveMutex();
            DEBUG("Pulling data for " << my_index);
            std::auto_ptr<Object> temp_data = 
                std::auto_ptr<Object>( source->get(my_index) );
            dataMutex.enterMutex();
            if (state == Untouched) {
                data = temp_data;
                state = Delivered;
            }
        } else {
            if ( state == Discarded ) 
                throw std::logic_error("Acquired image was discarded too early.");
            DEBUG("Waiting for image " << my_index);
            gotData.wait();
            DEBUG("Got image " << my_index);
        }
    }
}

template <typename Object>
Claim<Object> Slot<Object>::_workOn() {
    DEBUG("Locking for claim on " << my_index);
    ost::MutexLock lock(dataMutex);
    DEBUG("Locked for claim on " << my_index);
    if (claims > 0 || state >= Processed) {
        DEBUG("Invalid claim " << my_index);
        return Claim<Object>(NULL);
    } else {
        DEBUG("Claim might be valid, creating");
        Claim<Object> c(this);
        if (state == Untouched) {
            DEBUG("Fetching data for " << my_index);
            fetchData();
        }
        DEBUG("Returning valid claim for state " << state);
        return c;
    }
}

template <typename Object>
void Slot<Object>::insert(std::auto_ptr<Object> image) {
    ost::MutexLock lock(dataMutex);
    data = image;
    state = Delivered;
    gotData.signal();
}

}
}

#endif
