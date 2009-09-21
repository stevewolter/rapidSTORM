#ifndef DSTORM_TRANSMISSIONS_HIGHDEPTHIMAGE_IMPL
#define DSTORM_TRANSMISSIONS_HIGHDEPTHIMAGE_IMPL

#include "transmissions/HighDepthImage.h"
#include <cc++/thread.h>

using namespace std;

namespace dStorm {

template <typename BinLis, typename DiscLis>
void HighDepthImage<BinLis, DiscLis>::clean
    (const cimg_library::CImg<float>& src) 
{
    binningPub->clean(src);
    PROGRESS("Cleaning HDI");
    if (need_rediscretization) {
        PROGRESS("Discretization is dirty");
#ifndef USE_INTERMED_IMAGE
        float old_discretFactor = discretFactor;
#endif
        discretMax = currentMax;
        discretFactor = (depth-1) / currentMax;
        cimg_forXY(src, x, y) {
            if ( src(x,y) == 0 ) continue;
            SmoothedPixel 
#ifdef USE_INTERMED_IMAGE
                &oldVal = intermed(x,y),
#else
                oldVal = discrete( src(x,y), old_discretFactor ),
#endif
                val = discrete( src(x,y) );

            assert( int(val) < depth );
            if ( true || val != oldVal ) {
                discPub->updatePixel(x, y, oldVal, val );
#ifdef USE_INTERMED_IMAGE
                oldVal = val;
#endif
            }
        }
        need_rediscretization = false;
        dirty = 0;
        PROGRESS("Discretization clean");
    }
    PROGRESS("Cleaning histogram");
    discPub->clean();
    PROGRESS("Cleaned histogram");
}

template <typename BinLis, typename DiscLis>
void HighDepthImage<BinLis, DiscLis>::clear() {
    binningPub->clear();
#ifdef USE_INTERMED_IMAGE
    intermed.fill(0);
#endif
    currentMax = discretFactor = discretMax = 0;
    dirty = 0;
    need_rediscretization = true;
    discPub->clear();
}


}

#endif
