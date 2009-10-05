#ifndef CIMGBUFFER_SERIAL_IMAGE_SOURCE_H
#define CIMGBUFFER_SERIAL_IMAGE_SOURCE_H

#include "Source.h"
#include "Drain.h"
#include "ImageTraits.h"
#include <dStorm/helpers/thread.h>
#include <cassert>

namespace cimg_library {
    template <typename Pixel> class CImg;
}

namespace CImgBuffer {

template <typename PixelType>
class SerialImageSource 
    : public CImgBuffer::Source< cimg_library::CImg<PixelType> >
{
    /** The serializer controls multi-threaded access to the
     *  source. */
    ost::Mutex serializer;
    int current_image;
  protected:
    SerialImageSource( int flags ) 
        : CImgBuffer::Source< cimg_library::CImg<PixelType> >(flags),
          current_image(-1) {}
    virtual cimg_library::CImg<PixelType>* load() = 0;
    virtual cimg_library::CImg<PixelType>* fetch(int image_index);
};

template<typename Pixel>
cimg_library::CImg<Pixel>*
SerialImageSource<Pixel>::fetch(int image_index)
{
    ost::MutexLock lock(serializer);
    if ( this->pushTarget != NULL ) {
        int serial = std::max(current_image+1, int(this->roi_start));
        for (int image = serial; image < image_index; image++)
        {
            cimg_library::CImg<Pixel>* loaded = load();
            Management management;
            management = this->pushTarget->accept
                ( image - this->roi_start, 1, loaded );
            current_image = image;
            if ( management == Delete_objects )
                delete loaded;
        }
    }

    if ( current_image < image_index ) {
        assert( current_image == image_index - 1 );
        current_image = image_index;
        return load();
    } else
        return NULL;
}


}

#endif 
