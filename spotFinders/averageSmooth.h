#ifndef DSTORM_AVERAGE_SMOOTH_H
#define DSTORM_AVERAGE_SMOOTH_H

#include "image/fwd.h"

namespace dStorm {
namespace spotFinders {

/** Method smoothes the input image \c input, writes the
 *  smoothed result into the target image \c output (which
 *  may be the same, but must be large enough to hold the
 *  output), operating with the mask radii (half the width)
 *  given in the last arguments. */
template <typename InPix, typename OutPix>
void smoothByAverage( const Image<InPix,2>& input,
                      Image<OutPix,2>& output,
                      int mask_radius_x, int mask_radius_y );

}
}

#endif
