#ifndef DSTORM_GAUSSSMOOTHING_H
#define DSTORM_GAUSSSMOOTHING_H

#include <data-c++/Vector.h>
#include "SpotFinder.h"

namespace dStorm {
    class GaussSmoother : public SpotFinder {
      public:
        GaussSmoother (const Config &, int imw, int imh) throw ();

        void prepare( const Image &in ) throw();

      protected:
        data_cpp::Vector<int> xkern, ykern;
    };
}
#endif
