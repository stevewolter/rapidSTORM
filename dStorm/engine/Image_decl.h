#ifndef DSTORM_ENGINE_IMG_DECL_H
#define DSTORM_ENGINE_IMG_DECL_H

#include <dStorm/input/ImageTraits.h>

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
    /** Traits for input image for STORM engine. */
    typedef dStorm::input::Traits<Image> InputTraits;
    /** Output image type for STORM engine. */
    typedef cimg_library::CImg<SmoothedPixel> SmoothedImage;
}
}

#endif
