#define CIMGBUFFER_SLOT_CPP
#include <CImgBuffer/Slot.h>
#include <CImgBuffer/Image.h>

#include "Pixel.h"
#include <iostream>

using namespace std;
using namespace cimg_library;
using namespace ost;

namespace CImgBuffer {

typedef Source< Image<Pixel> > ImSource;
typedef Image<Pixel> Object;

template<>
Slot<Object>::Slot(ImSource &src, int index, const bool& discardable) 
    throw(std::exception)
: source(&src), my_index(index), mayDiscard(discardable),
  state(Untouched), dataMutex(), gotData(dataMutex)
{ 
    if ( ! source->pushes() && ! source->canBePulled() )
        throw std::logic_error
            ("Supplied source can neither push nor pull.");
}

template<>
Slot<Object>::Slot(const Slot<Object> &c) throw()
: source(c.source), my_index(c.my_index),
  claims(0), mayDiscard(c.mayDiscard),
  state(Untouched), dataMutex(), gotData(dataMutex)
{}

template<>
Slot<Object>& Slot<Object>::operator=(const Slot<Object> &c) throw() 
{
  source=(c.source); 
  my_index=(c.my_index),
  claims=(0); 
  state=(Untouched);
  return *this;
}

template<>
void Slot<Object>::finish() throw() {
    MutexLock lock(dataMutex);
    state = Finished;
    PROGRESS("Finished image " << my_index << " and may " 
             << ((mayDiscard) ? "" : "not ") << "discard");
    if ( mayDiscard && data.get() != NULL ) {
        data.reset(NULL);
    }
    PROGRESS("Discarded image " << my_index);
}

template<>
void Slot<Object>::clean() throw() {
    MutexLock lock(dataMutex);

    state = Untouched;
}

template<>
bool Slot<Object>::discard() throw() {
    MutexLock lock(dataMutex);
    bool hasDiscarded = (data.get() != NULL) && (state == Finished);
    if (state == Finished) data.reset( NULL );
    return hasDiscarded;
}

template<>
void Slot<Object>::fetchData() throw() {
    while ( data.get() == NULL ) {
        if ( source->canBePulled() ) {
            data = source->fetch(my_index);
            if (data.get() == NULL)
                state = Error;
        } else {
            gotData.wait();
            if (data.get() == NULL)
                state = Error;
        }
    }
}

template<>
Claim<Object> Slot<Object>::_workOn() throw() {
    MutexLock lock(dataMutex);
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

template<>
void Slot<Object>::insert(auto_ptr<Object> image) throw() {
    MutexLock lock(dataMutex);
    data = image;
    gotData.signal();
}

}
