#ifndef DSTORM_FILLHOLESMOOTHING_H
#define DSTORM_FILLHOLESMOOTHING_H

#include <data-c++/Vector.h>
#include "Image.h"
#include "SpotFinder.h"

namespace cimg_library {
    template <typename Pixel> class CImg;
}

namespace dStorm {
    class FillholeSmoother : public SpotFinder {
      private:
        std::auto_ptr<SmoothedImage> buffer[3];
        int rms1, rms2;

      public:
        FillholeSmoother (const Config &conf, int imw, int imh) throw () 
        : SpotFinder(conf, imw, imh),
          rms1(conf.reconstruction_mask_1()),
          rms2(conf.reconstruction_mask_2())
        {
            for (int i = 0; i < 3; i++)
                buffer[i].reset(new SmoothedImage(imw, imh));
        }
        ~FillholeSmoother() throw() {}

        void prepare( const Image &in ) throw();
    };
}
#endif
