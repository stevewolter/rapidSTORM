#ifndef DSTORM_RECONSTRUCTION_H
#define DSTORM_RECONSTRUCTION_H

#include <dStorm/Image_decl.h>

namespace dStorm {
    /** Perform reconstruction by dilation on the image src with mask
     *  \c mask. A 1 pixel wide border will be erased and set to 0 in
     *  the source and mask image. */
    template <typename PixelType>
    void
    ReconstructionByDilationII(dStorm::Image<PixelType,2>& src,
                               dStorm::Image<PixelType,2>& mask,
                               dStorm::Image<PixelType,2>& target)
        throw();
}

#endif
