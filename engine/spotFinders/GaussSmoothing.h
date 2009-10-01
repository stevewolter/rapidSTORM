#ifndef DSTORM_GAUSSSMOOTHING_H
#define DSTORM_GAUSSSMOOTHING_H

#include <data-c++/Vector.h>
#include "engine/SpotFinder.h"

namespace dStorm {
    class GaussSmoother : public SpotFinder {
      public:
        GaussSmoother (const Config &, int imw, int imh);

        void smooth( const Image &in );

      protected:
        data_cpp::Vector<int> xkern, ykern;
    };
}
#endif
