#ifndef DSTORM_SPALTTIEFPASS_H
#define DSTORM_SPALTTIEFPASS_H

#include <data-c++/Vector.h>
#include <dStorm/Image.h>
#include "SpotFinder.h"
#include "averageSmooth.h"

namespace cimg_library {
    template <typename Pixel> class CImg;
}

namespace dStorm {
    class Spalttiefpass : public SpotFinder {
      public:
        Spalttiefpass (const Config &conf, int imw, int imh) throw () 
        : SpotFinder(conf, imw, imh)
        {}

        void prepare( const Image &in ) throw() {
            smoothByAverage( in, *smoothed, msx, msy );
        }
    };
}
#endif
