#include "dilation_impl.h"
#include <dStorm/engine/Image.h>

namespace dStorm {

using dStorm::engine::StormPixel;
using dStorm::engine::SmoothedPixel;

template void rectangular_dilation<bool>(
        const Image<bool,2> &i,
        Image<bool,2> &t,
        const int mrx, const int mry,
        const int borderX, const int borderY);

template void rectangular_erosion<bool>(
        const Image<bool,2> &i,
        Image<bool,2> &t,
        const int mrx, const int mry,
        const int borderX, const int borderY);

template void rectangular_dilation<StormPixel,SmoothedPixel>(
        const Image<StormPixel,2> &i,
        Image<SmoothedPixel,2> &t,
        const int mrx, const int mry,
        const int borderX, const int borderY);

template void rectangular_erosion<StormPixel,SmoothedPixel>(
        const Image<StormPixel,2> &i,
        Image<SmoothedPixel,2> &t,
        const int mrx, const int mry,
        const int borderX, const int borderY);

}
