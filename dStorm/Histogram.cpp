#include "Histogram.h"
#include <stdint.h>
#include <limits>

using namespace std;

namespace dStorm {

const static int start = 1;
const static NormalizedHistogram::Out maxVal = (numeric_limits<NormalizedHistogram::Out>::max()-start);

void NormalizedHistogram::normalizeHistogram() throw() {
    Out newNormalization[depth];
    double fractions[depth];
    double accum = 0;

    for (unsigned int i = start; i < depth; i++) {
        if (histogram[i] != 0)
            accum += pow( histogram[i] / double(histogramSum), 
                          double(power) );
        fractions[i] = accum;
    }

    for (unsigned int i = start; i < depth; i++) {
        double q = (fractions[i] / accum);
        if (invert) q = 1 - q;
        newNormalization[i] = uint8_t(min<int>(max((int)(maxVal*q), 0), 
                                          numeric_limits<uint8_t>::max()));
        
        resultDirty |= (abs(newNormalization[i] - transition[i]) > 5);
    }

    if (resultDirty)
        for (unsigned int i = start; i < depth; i++)
            transition[i] = newNormalization[i];
    
    dirtyHistogramValues = 0;
}

}
