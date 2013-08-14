#ifndef DSTORM_THREED_INFO_TYPES_H
#define DSTORM_THREED_INFO_TYPES_H

#include "fwd.h"
#include <boost/icl/interval_set.hpp>

namespace dStorm {
namespace threed_info {

typedef double Sigma;
typedef double ZPosition;
typedef double SigmaDerivative;
typedef boost::icl::interval_set<ZPosition> ZRange;
typedef boost::icl::continuous_interval<ZPosition> ZInterval;

}
}
#endif
