#ifndef DSTORM_PSF_MODELS_H
#define DSTORM_PSF_MODELS_H

#include "DepthInfo3D.h"
#include "No3D.h"
#include <boost/mpl/vector.hpp>

namespace dStorm {
namespace gaussian_psf {

typedef boost::mpl::vector<DepthInfo3D,No3D> expressions;

}
}

#endif
