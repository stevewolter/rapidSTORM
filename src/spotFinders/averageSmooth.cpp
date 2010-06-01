#include "spotFinders/averageSmooth.h"
#include <dStorm/engine/Image.h>

using namespace dStorm::engine;

namespace dStorm {
namespace spotFinders {

#ifndef AVERAGE_BY_SLIDING_WINDOW
#define AVERAGE_BY_SEPARATION
#endif

#ifdef AVERAGE_BY_SEPARATION
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
    *target = accum;
    target += step;

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
}
#endif

template <>
void smoothByAverage<StormPixel,SmoothedPixel>(
    const Image<StormPixel,2>& input,
    Image<SmoothedPixel,2>& output,
    int xr, int yr )
{
#ifdef AVERAGE_BY_SLIDING_WINDOW
    int width = input.width, height = input.height;
    int t_width = width-2*xr, 
        t_height = height-2*yr;
    //int norm = (2*xr+1) * (2*yr+1);

    int runningSum = 0;
    for (int y = -yr; y <= yr; y++) {
        const unsigned short *line = input.ptr(0+xr, y+yr);
        for (int x = -xr; x <= xr; x++)
            runningSum += line[x];
    }
    output(0,0) = runningSum /*/ norm*/;

    int column = 0, row = 0;
    /* For each column but the first, slide to the right. */
    for (column = 0; column < t_width; ) {
        int dir = (column % 2) ? -1 : 1;

        /* For each row but the first, slide down if the column is even and
        * up otherwise. */
        for ( ; row != ((dir == 1) ? t_height-1 : 0); ) {
            row += dir;
            for (int i = 0; i < 2; i++) {
                int c_row = row + (2*i-1) * dir * yr + (i-1) * dir;

                const StormPixel *row_data = 
                    input.ptr(column+xr, c_row+yr);
                for (int dx = -xr; dx <= xr; dx++)
                runningSum += (2*i-1) * row_data[dx];
            }
            output(column, row) = runningSum /*/ norm*/;
        }

        /* Now slide to the side. */
        if (column++ < t_width - 1) {
            for (int i = 0; i < 2; i++) {
                int c_col = column + (2*i-1) * xr + i-1;

                for (int dy = -yr; dy <= yr; dy++)
                runningSum += (2*i-1) * input(c_col+xr, row+yr+dy);
            }
            output(column, row) = runningSum /*/ norm*/;
        }
    }
#elif defined(AVERAGE_BY_SEPARATION)
    for (int y = 0; y < int(input.height_in_pixels()); y++)
        averageLine<StormPixel,SmoothedPixel>
            ( input.ptr(0, y), 1, xr, input.width_in_pixels(),
                     output.ptr(xr, y) );
    for (int x = xr; x < int(output.width_in_pixels()-xr); x++)
        averageLine<SmoothedPixel,SmoothedPixel>
            ( output.ptr(x, 0), output.width_in_pixels(),
              yr, output.height_in_pixels(), 
                     output.ptr(x, yr) );

#ifdef NORMALIZE
    const unsigned int norm = (2*xr+1)*(2*yr+1), sz = output.size();
    for (unsigned int i = 0; i < sz; i++)
        output.data[i] /= norm;
#endif
#endif
}

}
}
