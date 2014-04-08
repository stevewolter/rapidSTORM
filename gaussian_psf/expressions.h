#ifndef DSTORM_PSF_MODELS_H
#define DSTORM_PSF_MODELS_H

#include "gaussian_psf/DepthInfo3D.h"
#include "gaussian_psf/No3D.h"
#include <boost/mpl/vector.hpp>

namespace dStorm {
namespace gaussian_psf {

typedef boost::mpl::vector<DepthInfo3D,No3D> expressions;

}
}

#endif
