#ifndef DSTORM_FITTER_MSD_PSF_PARAMETERS_H
#define DSTORM_FITTER_MSD_PSF_PARAMETERS_H

#include <nonlinfit/Xs.h>
#include "LengthUnit.h"



namespace dStorm {
namespace measured_psf {

using namespace boost::units;

typedef nonlinfit::Xs< 0,LengthUnit > XPosition;
typedef nonlinfit::Xs< 1,LengthUnit > YPosition;
typedef nonlinfit::Xs< 2,LengthUnit > ZPosition;

template <int _Dimension> struct Mean {
    static const int Dimension = _Dimension;
    typedef LengthUnit Unit;
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
