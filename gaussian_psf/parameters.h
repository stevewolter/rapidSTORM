#ifndef DSTORM_FITTER_PSF_PARAMETERS_H
#define DSTORM_FITTER_PSF_PARAMETERS_H

#include "fwd.h"
#include <nonlinfit/Xs.h>

namespace dStorm {
namespace gaussian_psf {

typedef nonlinfit::Xs< 0 > XPosition;
typedef nonlinfit::Xs< 1 > YPosition;

template <int _Dimension> struct Mean {
    static const int Dimension = _Dimension;
};

struct MeanZ {};
template <int _Dimension> struct BestSigma {
    static const int Dimension = _Dimension;
};
template <int _Dimension, int Power> struct DeltaSigma {
    static const int Dimension = _Dimension;
};
template <int _Dimension> struct ZPosition {
    static const int Dimension = _Dimension;
};
struct Amplitude {};
struct Prefactor {};

}
}

#endif
