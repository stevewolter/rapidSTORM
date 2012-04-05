#ifndef DSTORM_THREED_INFO_DEPTH_RANGE_H
#define DSTORM_THREED_INFO_DEPTH_RANGE_H

#include <dStorm/traits/DepthInfo.h>
#include <boost/icl/continuous_interval.hpp>
#include <dStorm/types/samplepos.h>

namespace dStorm {
namespace traits {

typedef boost::icl::continuous_interval< samplepos::Scalar >
    ZRange;

boost::optional<ZRange> get_z_range( const DepthInfo& );
boost::optional<ZRange> merge_z_range( const boost::optional<ZRange>&, const boost::optional<ZRange>& );

}
}

#endif
