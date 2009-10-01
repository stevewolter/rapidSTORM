#ifndef DSTORM_MEDIANSMOOTHER_H
#define DSTORM_MEDIANSMOOTHER_H

#include <data-c++/Vector.h>
#include "engine/SpotFinder.h"
#include <CImg.h>

namespace cimg_library {
    template <typename Pixel> class CImg;
}

namespace dStorm {
    class MedianSmoother : public SpotFinder {
      private:
        void naiveMedian(const Image &in, SmoothedImage& out, 
                         int mw, int mh);
        
        void (*ahmad)(const Image &in, SmoothedImage& out, int mw, int mh)
;
        void chooseAhmad(int msx, int msy);

      public:
        MedianSmoother (const Config &conf, int imw, int imh) 
        : SpotFinder(conf, imw, imh)
        {
            chooseAhmad(msx, msy);
        }

        void smooth( const Image &in ) {
            ahmad( in, *smoothed, 2*msx+1, 2*msy+1 );
        }
    };
}
#endif
