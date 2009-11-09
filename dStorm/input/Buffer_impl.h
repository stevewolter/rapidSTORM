#ifndef DSTORM_INPUT_BUFFER_IMPL_H
#define DSTORM_INPUT_BUFFER_IMPL_H

#include <cassert>
#include <dStorm/helpers/thread.h>
#include <limits>

#include "Buffer.h"
#include "Source.h"
#include "Slot.h"

using namespace std;

namespace dStorm {
namespace input {

template <typename Object>
void Buffer<Object>::receive_number_of_objects(int present_images) 
{
    LOCKING("Receiving " << present_images << " of objects");
    ost::MutexLock lock(initialization);
    /* This construction method is equivalent to calling 
     * push_back(Slot<Object>(*source, i, mayDiscard)) for each i,
     * but uses fewer allocations. I cannot recall why the allocation
     * count here is important. */
    Slot<Object> exemplar(*source, -1, mayDiscard);
    LOCKING("Resizing Buffer vector to " << present_images);
    resize(present_images, exemplar);
    LOCKING("Resized Buffer vector");
    int index = 0;
    for (typename data_cpp::Vector< Slot<Object> >::iterator
                  i = this->data_cpp::Vector< Slot<Object> >::begin();
                  i != this->data_cpp::Vector< Slot<Object> >::end(); i++ )
        i->setIndex(index++);

    initialized = true;
    finished_initialization.signal();
}

template<typename Object>
void Buffer<Object>::init()
{
    mayDiscard = false;

    if ( source->pull_length() )
        receive_number_of_objects( source->number_of_objects() );

    if (source->pushes_concurrently()) 
        source->startPushing(this);
    else
        source->allowPushing(this);
    LOCKING("Created image buffer");
}

template<typename Object>
Buffer<Object>::Buffer(const Config &c) 

: finished_initialization(initialization),
  initialized(false)
{
    std::auto_ptr< BaseSource > base = c.makeImageSource();
    Source<Object>* src = dynamic_cast< Source<Object>* >(base.get());
    if ( src == NULL ) {
        throw std::logic_error("Source of incorrect type supplied to "
                               "buffer.");
    } else {
        source.reset( src );
        base.release();
        init();
    }
}

template<typename Object>
Buffer<Object>::Buffer(std::auto_ptr< Source<Object> > src) 

: finished_initialization(initialization),
  initialized(false)
{
    assert( src.get() != NULL );
    source = src;
    this->init();
}

template<typename Object>
void Buffer<Object>::signal_end_of_acquisition() {
    if (source->pushes()) source->stopPushing(this);
}

template<typename Object>
Buffer<Object>::~Buffer() {
    STATUS("Destructing " << (void*)this);
    signal_end_of_acquisition();
    STATUS("Destructed " << this);
}

template<typename Object>
void Buffer<Object>::setDiscardingLicense(bool mD) {
    if (mayDiscard == false && mD == true) {
        mayDiscard = true;
        /* If the discarding license is set to true, discard all images. */
        for (typename data_cpp::Vector< Slot<Object> >::iterator 
                 i = this->begin(); i != this->end(); i++) 
        {
            i->discard();
        }
    } else
        mayDiscard = mD;
}

template<typename Object>
void Buffer<Object>::makeAllUntouched() {
    for(typename data_cpp::Vector< Slot<Object> >::iterator 
                 i = this->begin(); i != this->end(); i++)
        i->clean();
}

template<typename Object>
Management
Buffer<Object>::accept(int index, int number, Object* i)

{
    assert( i == NULL || number == 1 );
    for ( int j = 0; j < number; j++ )
        (*this)[index].insert( std::auto_ptr<Object>(i) );
    return Keeps_objects;
}

template<typename Object>
simparm::Node& Buffer<Object>::getConfig()
{
    return *source;
}

}
}

#endif
