#ifndef DSTORM_RECONSTRUCTION_H
#define DSTORM_RECONSTRUCTION_H

#include <dStorm/image/fwd.h>

namespace dStorm {
namespace image {
    /** Perform reconstruction by dilation on the image src with mask
     *  \c mask. A 1 pixel wide border will be erased and set to 0 in
     *  the source and mask image. */
    template <typename PixelType>
    void reconstruction_by_dilation(
        Image<PixelType,2>& src,
        Image<PixelType,2>& mask,
        Image<PixelType,2>& target);

}
}

#endif
