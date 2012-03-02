#ifndef DSTORM_PSF_MODELS_H
#define DSTORM_PSF_MODELS_H

#include "Polynomial3D.h"
#include "No3D.h"
#include <boost/mpl/vector.hpp>

namespace dStorm {
namespace guf {
namespace PSF {

typedef boost::mpl::vector<Polynomial3D,No3D> expressions;

}
}
}

#endif
