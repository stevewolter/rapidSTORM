#ifndef DSTORM_AVERAGE_SMOOTH_H
#define DSTORM_AVERAGE_SMOOTH_H

namespace cimg_library {
    template <typename T> class CImg;
}

namespace dStorm {

/** Method smoothes the input image \c input, writes the
 *  smoothed result into the target image \c output (which
 *  may be the same, but must be large enough to hold the
 *  output), operating with the mask radii (half the width)
 *  given in the last arguments. */
template <typename InPix, typename OutPix>
void smoothByAverage( const cimg_library::CImg<InPix>& input,
                      cimg_library::CImg<OutPix>& output,
                      int mask_radius_x, int mask_radius_y );

}

#endif
