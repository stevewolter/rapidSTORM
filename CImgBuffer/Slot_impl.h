#ifndef CIMGBUFFER_SLOT_IMPL_H
#define CIMGBUFFER_SLOT_IMPL_H
#include <CImgBuffer/Slot.h>

#include <iostream>

namespace CImgBuffer {

template <typename Object>
Slot<Object>::Slot(Source<Object> &src,int index, const bool& discardable) 

: source(&src), my_index(index), mayDiscard(discardable),
  state(Untouched), dataMutex(), gotData(dataMutex)
{ 
    PROGRESS("Initial discarding license: " << discardable);
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
    state = Finished;
    PROGRESS("Finished image " << my_index << " and may " 
             << ((mayDiscard) ? "" : "not ") << "discard");
    if ( mayDiscard && data.get() != NULL ) {
        data.reset(NULL);
        PROGRESS("Discarded image " << my_index);
    }
}

template <typename Object>
void Slot<Object>::clean() {
    ost::MutexLock lock(dataMutex);

    state = Untouched;
}

template <typename Object>
bool Slot<Object>::discard() {
    ost::MutexLock lock(dataMutex);
    bool hasDiscarded = (data.get() != NULL) && (state == Finished);
    if (state == Finished) data.reset( NULL );
    return hasDiscarded;
}

template <typename Object>
void Slot<Object>::fetchData() {
    while ( state != Error && data.get() == NULL ) {
        if ( source->canBePulled() ) {
            dataMutex.leaveMutex();
            std::auto_ptr<Object> temp_data = 
                std::auto_ptr<Object>( source->get(my_index) );
            dataMutex.enterMutex();
            if (data.get() == NULL)
                data = temp_data;
            if (data.get() == NULL)
                state = Error;
        } else {
            if ( state == Finished ) 
                throw std::logic_error("Acquired image was discarded too early.");
            PROGRESS("Waiting for image " << my_index);
            gotData.wait();
            PROGRESS("Got image " << my_index);
            if (data.get() == NULL)
                state = Error;
        }
    }
}

template <typename Object>
Claim<Object> Slot<Object>::_workOn() {
    ost::MutexLock lock(dataMutex);
    if (state == InWork || state == Finished || state == Error)
        return Claim<Object>(NULL);
    else {
        state = InWork;
        if (data.get() == NULL) {
            fetchData();
            if (state == Error)
                return Claim<Object>(NULL);
        }
        return Claim<Object>(this);
    }
}

template <typename Object>
void Slot<Object>::insert(auto_ptr<Object> image) {
    ost::MutexLock lock(dataMutex);
    data = image;
    gotData.signal();
}

}

#endif
