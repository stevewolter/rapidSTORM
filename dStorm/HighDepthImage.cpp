#include "HighDepthImage.h"
#include <cc++/thread.h>

using namespace std;

namespace dStorm {
void HighDepthImage::clean(const cimg_library::CImg<float>& src) throw() {
    bool wasDirty = dirty;
    PROGRESS("Cleaning HDI");
    if (dirty) {
        PROGRESS("Discretization is dirty");
        discretMax = currentMax;
        discretFactor = (depth-1) / currentMax;
        cimg_forXY(src, x, y) {
            unsigned short val = 
                (unsigned short)floor(discretFactor * src(x,y));
            assert( val < depth );
            image(x,y) = val;
        }
        dirty = false;
        PROGRESS("Discretization clean");
        if (histogram) {
            PROGRESS("Reomputing histogram");
            histogram->compute(image);
            PROGRESS("Reomputed histogram");
        }
    }
    if (histogram) {
        PROGRESS("Cleaning histogram");
        histogram->cleanHistogram();
        wasDirty |= histogram->isDirty();
        PROGRESS("Cleaned histogram");
    }
    if (wasDirty && target) {
        PROGRESS("Recomputing target image");
        target->recompute(image);
        PROGRESS("Recomputed target image");
    }
}

void HighDepthImage::clear() throw() {
    image.fill(0);
    currentMax = discretFactor = discretMax = 0;
    if (histogram) histogram->clear(image.width*image.height);
    if (target) target->recompute(image);
}


}
