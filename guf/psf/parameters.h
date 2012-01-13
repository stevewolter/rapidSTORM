#ifndef DSTORM_FITTER_PSF_PARAMETERS_H
#define DSTORM_FITTER_PSF_PARAMETERS_H

#include "fwd.h"
#include <nonlinfit/Xs.h>

namespace dStorm {
namespace guf {
namespace PSF {

using namespace boost::units;

typedef nonlinfit::Xs< 0,LengthUnit > XPosition;
typedef nonlinfit::Xs< 1,LengthUnit > YPosition;

template <int _Dimension> struct Mean {
    static const int Dimension = _Dimension;
    typedef LengthUnit Unit;
};

struct MeanZ { typedef Micrometers Unit; };
typedef nonlinfit::Xs< 2,LengthUnit > ZPosition;
template <int _Dimension> struct BestSigma {
    typedef si::dimensionless Unit;
    static const int Dimension = _Dimension;
};
template <int _Dimension> struct DeltaSigma {
    static const int Dimension = _Dimension;
    typedef 
        power_typeof_helper<
            power10< si::length, -6 >::type,
            static_rational<-2>
        >::type Unit;
};
template <int _Dimension> struct ZOffset {
    static const int Dimension = _Dimension;
    typedef Micrometers Unit;
};
struct Amplitude {
    typedef si::dimensionless Unit;
};
struct Prefactor {
    typedef si::dimensionless Unit;
};
struct Wavelength {
    typedef Micrometers Unit;
};

}
}
}

#endif
