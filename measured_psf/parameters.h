#ifndef DSTORM_FITTER_PSF_PARAMETERS_H
#define DSTORM_FITTER_PSF_PARAMETERS_H

#include "fwd.h"
#include <nonlinfit/Xs.h>

namespace dStorm {
namespace measured_psf {

using namespace boost::units;

typedef nonlinfit::Xs< 0,LengthUnit > XPosition;
typedef nonlinfit::Xs< 1,LengthUnit > YPosition;

template <int _Dimension> struct Mean {
    static const int Dimension = _Dimension;
    typedef LengthUnit Unit;
};

struct MeanZ { typedef Micrometers Unit; };
template <int _Dimension> struct BestSigma {
    typedef Micrometers Unit;
    static const int Dimension = _Dimension;
};
template <int _Dimension, int Power> struct DeltaSigma {
    static const int Dimension = _Dimension;
    typedef Micrometers Unit;
};
template <int _Dimension> struct ZPosition {
    static const int Dimension = _Dimension;
    typedef Micrometers Unit;
};
struct Amplitude {
    typedef si::dimensionless Unit;
};
struct Prefactor {
    typedef si::dimensionless Unit;
};

}
}

#endif
