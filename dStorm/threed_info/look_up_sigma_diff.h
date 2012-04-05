#ifndef DSTORM_THREED_INFO_DEPTH_RANGE_H
#define DSTORM_THREED_INFO_DEPTH_RANGE_H

#include "DepthInfo.h"
#include <boost/icl/continuous_interval.hpp>
#include <dStorm/types/samplepos.h>

namespace dStorm {
namespace threed_info {

samplepos::Scalar look_up_sigma_diff( 
    const DepthInfo& o, 
    boost::units::quantity<boost::units::si::length> sigma_diff );

}
}

#endif
