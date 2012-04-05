#ifndef DSTORM_THREED_INFO_DEPTH_RANGE_H
#define DSTORM_THREED_INFO_DEPTH_RANGE_H

#include <dStorm/traits/DepthInfo.h>
#include <boost/icl/continuous_interval.hpp>
#include <dStorm/types/samplepos.h>

namespace dStorm {
namespace traits {

samplepos::Scalar look_up_sigma_diff( 
    const traits::DepthInfo& o, 
    boost::units::quantity<boost::units::si::length> sigma_diff );

}
}

#endif
