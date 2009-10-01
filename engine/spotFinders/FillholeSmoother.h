#ifndef DSTORM_FILLHOLESMOOTHING_H
#define DSTORM_FILLHOLESMOOTHING_H

#include <data-c++/Vector.h>
#include "engine/Image.h"
#include "engine/SpotFinder.h"
#include "engine/Config.h"

namespace cimg_library {
    template <typename Pixel> class CImg;
}

namespace dStorm {
    class FillholeSmoother : public SpotFinder {
      private:
        std::auto_ptr<SmoothedImage> buffer[3];
        int rms1, rms2;

      public:
        FillholeSmoother (const Config &conf, int imw, int imh);
        ~FillholeSmoother() {}

        void smooth( const Image &in );
    };
}
#endif
