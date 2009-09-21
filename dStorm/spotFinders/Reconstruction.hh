#ifndef DSTORM_RECONSTRUCTION_H
#define DSTORM_RECONSTRUCTION_H

namespace cimg_library {
    template <typename Pixel> class CImg;
}

namespace dStorm {
    /** Perform reconstruction by dilation on the image src with mask
     *  \c mask. A 1 pixel wide border will be erased and set to 0 in
     *  the source and mask image. */
    template <typename PixelType> void
    ReconstructionByDilationII(cimg_library::CImg<PixelType>& src,
                               cimg_library::CImg<PixelType>& mask,
                               cimg_library::CImg<PixelType>& target)
        throw();
}

#endif
