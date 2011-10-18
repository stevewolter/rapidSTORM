#ifndef DSTORM_ENGINE_IMG_DECL_H
#define DSTORM_ENGINE_IMG_DECL_H

#include "../Image_decl.h"

namespace dStorm {
namespace engine {
    /** Input pixel type for STORM engine. */
    typedef unsigned short StormPixel;
    /** Output pixel type for STORM engine. */
    typedef unsigned int SmoothedPixel;
    /** Input image type for STORM engine */
    typedef dStorm::Image<StormPixel,2> Image2D;
    typedef dStorm::Image<StormPixel,3> Image;
    /** Output image type for STORM engine. */
    typedef dStorm::Image<SmoothedPixel,2> SmoothedImage;

    typedef input::Traits<Image> InputTraits;
}

}

#endif
