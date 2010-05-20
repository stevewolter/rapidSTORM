#ifndef DSTORM_ENGINE_IMG_DECL_H
#define DSTORM_ENGINE_IMG_DECL_H

#include <dStorm/Image_decl.h>

namespace dStorm {
namespace engine {
    /** Input pixel type for STORM engine. */
    typedef unsigned short StormPixel;
    /** Output pixel type for STORM engine. */
    typedef unsigned int SmoothedPixel;
    /** Basic functions of input image type for
     *  STORM engine */
    typedef dStorm::BaseImage<StormPixel,2> BaseImage;
    /** Input image type for STORM engine */
    typedef dStorm::Image<StormPixel,2> Image;
    /** Output image type for STORM engine. */
    typedef dStorm::Image<SmoothedPixel,2> SmoothedImage;

    typedef input::Traits<Image> InputTraits;
}

}

#endif
