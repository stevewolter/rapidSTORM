#ifndef DSTORM_IMAGE_H
#define DSTORM_IMAGE_H

#ifdef HAVE_LIBMAGICK__
#define cimg_use_magick
#endif

namespace cimg_library {
    template <typename PixelType> class CImg;
};

namespace dStorm {
    /** Input pixel type for STORM engine. */
    typedef unsigned short StormPixel;
    /** Output pixel type for STORM engine. */
    typedef unsigned int SmoothedPixel;
    /** Input image type for STORM engine. */
    typedef cimg_library::CImg<StormPixel> Image;
    /** Output image type for STORM engine. */
    typedef cimg_library::CImg<SmoothedPixel> SmoothedImage;
}

#endif
