#include "Histogram.h"
#include <stdint.h>
#include <limits>

using namespace std;

namespace dStorm {

template <typename UNL, typename NL>
void NormalizedHistogram<UNL, NL>::normalizeHistogram() {
    const static int start = 1;
    const Out maxVal = (out_depth-start);

    double fractions[in_depth];
    double accum = 0;
    unsigned long int unused_pixels = 0;
    for (int i = 0; i < start; i++)
        unused_pixels += histogram[i];
    unsigned long used_histogram_pixels = histogramSum - unused_pixels;

    if ( used_histogram_pixels == 0U ) {
        dirtyHistogramValues = 0;
        return;
    }

    for (unsigned int i = start; i < in_depth; i++) {
        if (histogram[i] != 0)
            accum += pow( 
                double(histogram[i]) / used_histogram_pixels, 
                double(power) );
        fractions[i] = accum;
    }

    for (unsigned int i = start; i < in_depth; i++) {
        double q = (fractions[i] / accum);
        int newValue = min<int>(max<int>(maxVal*q, 0)+start, out_depth);
        int &oldValue = transition[i];
        
        if ( abs( newValue - oldValue ) > 5 ) {
            for ( ValueNode* j = value_lists[i].next; 
                             j != &value_lists[i]; j = j->next)
                normalized->updatePixel( j->x, j->y, oldValue, newValue );
            oldValue = newValue;
        }
    }

    dirtyHistogramValues = 0;
}

}
