#ifndef DSTORM_MEDIANSMOOTHER_H
#define DSTORM_MEDIANSMOOTHER_H

#include <data-c++/Vector.h>
#include <dStorm/Image.h>
#include "SpotFinder.h"
#include <CImg.h>

namespace cimg_library {
    template <typename Pixel> class CImg;
}

namespace dStorm {
    class MedianSmoother : public SpotFinder {
      private:
        void naiveMedian(const Image &in, SmoothedImage& out, 
                         int mw, int mh) throw();
        
        void (*ahmad)(const Image &in, SmoothedImage& out, int mw, int mh)
            throw();
        void chooseAhmad(int msx, int msy) throw();

      public:
        MedianSmoother (const Config &conf, int imw, int imh) throw () 
        : SpotFinder(conf, imw, imh)
        {
            chooseAhmad(msx, msy);
        }

        void prepare( const Image &in ) throw() {
            //naiveMedian( in, *smoothed, 2*msx+1, 2*msy+1 );
            //*smoothed = in;
            //smoothed->blur_median( (msx+msy)+1 );
            ahmad( in, *smoothed, 2*msx+1, 2*msy+1 );
        }
    };
}
#endif
