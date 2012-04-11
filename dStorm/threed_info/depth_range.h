#ifndef DSTORM_THREED_INFO_DEPTH_RANGE_H
#define DSTORM_THREED_INFO_DEPTH_RANGE_H

#include <dStorm/threed_info/DepthInfo.h>
#include <boost/icl/continuous_interval.hpp>
#include <dStorm/types/samplepos.h>
#include "types.h"

namespace dStorm {
namespace threed_info {

ZRange get_z_range( const DepthInfo& );

}
}

#endif
