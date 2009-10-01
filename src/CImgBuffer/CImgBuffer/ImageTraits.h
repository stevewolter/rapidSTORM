#ifndef CIMGBUFFER_IMAGETRAITS_H
#define CIMGBUFFER_IMAGETRAITS_H

#include <CImgBuffer/Traits.h>

namespace cimg_library {
    template <typename PixelType> class CImg;
};

namespace CImgBuffer {

/** The Traits class partial specialization for images
 *  provides methods to determine the image coordinates. */
template <typename PixelType>
class Traits< cimg_library::CImg<PixelType> > {
    public:
    virtual int dimx() const = 0;
    virtual int dimy() const = 0;
    virtual int dimz() const { return 1; }
    virtual int dimv() const { return 1; }
};
};

#endif
