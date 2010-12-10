#include <cassert>
#include <CImgBuffer/Buffer.h>
#include <CImgBuffer/Source.h>
#include <foreach.h>
#include <cc++/thread.h>
#include <CImgBuffer/Image.h>
#include <CImgBuffer/Slot.h>
#include <limits>

#include "Pixel.h"

using namespace std;

namespace CImgBuffer {

template<>
void Buffer<Object>::init(int fi, int li) throw(std::exception)
{
    mayDiscard = true;

    int present_images = this->source->image_quantity();
    if (li < 0 || li > present_images)
        li = present_images;
    if (fi > li) fi = li;
    present_images = li - fi;

    /* This construction method is equivalent to calling 
     * push_back(Slot<Object>(*source, i, mayDiscard)) for each i,
     * but uses fewer allocations. I cannot recall why the allocation
     * count here is important. */
    Slot<Object> exemplar(*source, -1, mayDiscard);
    LOCKING("Resizing Buffer vector to " << present_images);
    resize(present_images, exemplar);
    LOCKING("Resized Buffer vector");
    int index = 0;
    foreach( i, data_cpp::Vector< Slot<Object> >, *this )
        i->setIndex(index++);

    if (source->pushes()) source->startPushing(this);
}

template<>
Buffer<Object>::Buffer(const Config<Object> &c) 
    throw(std::exception)
{
    source = c.makeImageSource();
    init( c.firstImage(), c.lastImage());
}

template<>
Buffer<Object>::Buffer(std::auto_ptr< Source<Object> > src) 
    throw(std::exception)
{
    source = src;
    init(0, std::numeric_limits<int>::max());
}

template<>
Buffer<Object>::~Buffer() throw() {
    STATUS("Destructing " << (void*)this);
    if (source->pushes()) source->stopPushing(this);
    STATUS("Destructed " << this);
}

template<>
int Buffer<Object>::dimx() const throw() { return source->dimx(); }
template<>
int Buffer<Object>::dimy() const throw() { return source->dimy(); }

template<>
void Buffer<Object>::setDiscardingLicense(bool mD) throw() {
    if (mayDiscard == false && mD == true) {
        mayDiscard = true;
        /* If the discarding license is set to true, discard all images. Since
         * the images are filled sequentially, abort once no images are discarded. */
        for (iterator i = begin(); i != end(); i++) {
            if ( ! i->discard() ) 
                continue;
        }
    } else
        mayDiscard = mD;
}

template<>
void Buffer<Object>::makeAllUntouched() throw() {
    for(iterator it = begin(); it != end(); it++)
        it->clean();
}

template<>
void Buffer<Object>::accept(int index, 
                                    std::auto_ptr<Object> i)
throw()
{
    (*this)[index].insert(i);
}

template<>
ConfigNode& Buffer<Object>::getConfig() throw()
{
    return source->getConfig();
}

}
