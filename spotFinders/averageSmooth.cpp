#include "spotFinders/averageSmooth.h"
#include "engine/Image.h"

using namespace dStorm::engine;

namespace dStorm {
namespace spotFinders {

template <typename InPix, typename OutPix>
void averageLine( const InPix *array, int step, int radius, 
                  int size, OutPix *target) 
{
    int width = radius*2+1;
    /* This array is a ring buffer for the original pixel
     * values, which must be removed from accum in sequence. */
    InPix removal[width];
    /* Ring buffer index. */
    int removal_index = 0;
    OutPix accum = 0;
    const InPix *scan = array;

    for (int i = 0; i < width; i++) {
        removal[removal_index] = *scan;
        accum += *scan;
        scan += step;
        removal_index++;
        size--;
    }
    removal_index = 0;
    for (int i = 0; i < radius+1; i++) {
        *target = accum;
        target += step;
    }

    while (size > 0) {
        accum += *scan;
        accum -= removal[removal_index];
        removal[removal_index] = *scan;
        *target = accum;

        removal_index++;
        if (removal_index >= width) removal_index = 0;
        target += step;
        scan += step;
        size--;
    }

    for (int i = 0; i < radius; i++) {
        *target = accum;
        target += step;
    }
}

template <typename InPix, typename OutPix>
void smoothByAverage(
    const Image<InPix,2>& input,
    Image<OutPix,2>& output,
    int xr, int yr )
{
    /* Accumulate in X direction */
    for (int y = 0; y < int(input.height_in_pixels()); y++)
        averageLine<InPix,OutPix>(
            input.ptr(0, y),
            input.get_offsets().x(),
            xr,
            input.width_in_pixels(),
            output.ptr(0, y) 
        );
    /* Accumulate in Y direction */
    for (int x = 0; x < int(output.width_in_pixels()); x++)
        averageLine<OutPix,OutPix>(
            output.ptr(x, 0),
            output.get_offsets().y(),
            yr, 
            output.height_in_pixels(), 
            output.ptr(x, 0) 
        );

    /* Divide by window size */
    const unsigned int norm = (2*xr+1)*(2*yr+1), sz = output.size_in_pixels();
    for (unsigned int i = 0; i < sz; i++)
        output[i] /= norm;
}

template void smoothByAverage< StormPixel, SmoothedPixel >( const Image<StormPixel,2>&, Image<SmoothedPixel,2>&, int, int );

}
}
