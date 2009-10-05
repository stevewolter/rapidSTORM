#ifndef DSTORM_EROSIONSMOOTHER_H
#define DSTORM_EROSIONSMOOTHER_H

#include <data-c++/Vector.h>
#include <dStorm/Image.h>
#include "SpotFinder.h"
#include <CImg.h>

namespace cimg_library {
    template <typename Pixel> class CImg;
}

namespace dStorm {
    class ErosionSmoother : public SpotFinder {
      private:
        cimg_library::CImg<bool> mask;
      public:
        ErosionSmoother (const Config &conf, int imw, int imh) 
        : SpotFinder(conf, imw, imh),
          mask(msx+(1-msx%2), msy+(1-msy%2))
        {
            mask.fill(true);
        }
        ~ErosionSmoother() {}

        void smooth( const Image &in ) {
            *smoothed = in;
            smoothed->erode<bool>(mask);
        }
    };
}
#endif
