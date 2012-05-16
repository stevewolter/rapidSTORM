#ifndef DSTORM_FITTER_MSD_PSF_PARAMETERS_H
#define DSTORM_FITTER_MSD_PSF_PARAMETERS_H

#include <nonlinfit/Xs.h>
#include "LengthUnit.h"
#include "gaussian_psf/parameters.h"

namespace dStorm {
namespace measured_psf {

using namespace boost::units;
using gaussian_psf::Mean;
using gaussian_psf::Amplitude;
using gaussian_psf::Prefactor;

typedef nonlinfit::Xs< 0,LengthUnit > XPosition;
typedef nonlinfit::Xs< 1,LengthUnit > YPosition;
typedef nonlinfit::Xs< 2,LengthUnit > ZPosition;

}
}

#endif
