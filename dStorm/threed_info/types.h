#ifndef DSTORM_THREED_INFO_TYPES_H
#define DSTORM_THREED_INFO_TYPES_H

#include "fwd.h"
#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/icl/interval_set.hpp>
#include <dStorm/units/icl.h>

namespace dStorm {
namespace threed_info {

using boost::units::quantity;
namespace si = boost::units::si;

typedef quantity<si::length,float> Sigma;
typedef quantity<si::length,float> ZPosition;
typedef double SigmaDerivative;
typedef boost::icl::interval_set<ZPosition> ZRange;
typedef boost::icl::continuous_interval<ZPosition> ZInterval;

}
}
#endif
