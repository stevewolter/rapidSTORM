#ifndef DSTORM_ENGINE_IMG_DECL_H
#define DSTORM_ENGINE_IMG_DECL_H

namespace cimg_library {
    template <typename PixelType> class CImg;
};

namespace dStorm {
namespace engine {
    /** Input pixel type for STORM engine. */
    typedef unsigned short StormPixel;
    /** Output pixel type for STORM engine. */
    typedef unsigned int SmoothedPixel;
    /** Input image type for STORM engine. */
    typedef cimg_library::CImg<StormPixel> Image;
    /** Output image type for STORM engine. */
    typedef cimg_library::CImg<SmoothedPixel> SmoothedImage;
}
}

#endif
