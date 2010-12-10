#ifndef DSTORM_IMAGE_H
#define DSTORM_IMAGE_H

#ifdef HAVE_LIBMAGICK__
#define cimg_use_magick
#endif

namespace cimg_library {
    template <typename PixelType> class CImg;
};

namespace CImgBuffer {
    template <typename PixelType> class Image;
};

namespace dStorm {
    typedef unsigned short StormPixel;
    typedef unsigned int SmoothedPixel;
    typedef CImgBuffer::Image<StormPixel> Image;
    typedef cimg_library::CImg<SmoothedPixel> SmoothedImage;
}

#endif
