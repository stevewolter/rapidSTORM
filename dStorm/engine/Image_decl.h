#ifndef DSTORM_ENGINE_IMG_DECL_H
#define DSTORM_ENGINE_IMG_DECL_H

#include <dStorm/input/Traits.h>
#include <dStorm/image/fwd.h>

namespace dStorm {
namespace engine {
    /** Input pixel type for STORM engine. */
    typedef unsigned short StormPixel;
    /** Output pixel type for STORM engine. */
    typedef unsigned int SmoothedPixel;
    /** Input image type for STORM engine */
    typedef dStorm::Image<StormPixel,2> Image2D;
    class ImageStack;
    /** Output image type for STORM engine. */
    typedef dStorm::Image<SmoothedPixel,2> SmoothedImage;

    typedef input::Traits<ImageStack> InputTraits;
}

}

#endif
