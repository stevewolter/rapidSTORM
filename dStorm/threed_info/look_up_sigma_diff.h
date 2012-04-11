#ifndef DSTORM_THREED_INFO_LOOK_UP_SIGMA_DIFF_H
#define DSTORM_THREED_INFO_LOOK_UP_SIGMA_DIFF_H

#include "DepthInfo.h"
#include <boost/icl/continuous_interval.hpp>
#include <dStorm/types/samplepos.h>
#include <dStorm/Direction.h>
#include <boost/optional/optional.hpp>
#include "types.h"

namespace dStorm {
namespace threed_info {

struct SigmaDiffLookup {
    const DepthInfo &a, &b;
    const Direction a_dim, b_dim;
public:
    SigmaDiffLookup( 
        const DepthInfo& minuend,
        Direction minuend_dimension,
        const DepthInfo& subtrahend,
        Direction subtrahend_dimension )
        : a( minuend ), b( subtrahend ), 
          a_dim(minuend_dimension), b_dim( subtrahend_dimension ) {}

    SigmaDiffLookup( const DepthInfo& o )
        : a( o ), b( o ), a_dim( Direction_X ), b_dim( Direction_Y ) {}


    Sigma get_sigma_diff( ZPosition ) const;
    boost::optional<ZPosition> look_up_sigma_diff( Sigma sigma_diff, Sigma precision ) const;
    ZRange get_z_range() const;
};

boost::optional<ZPosition> look_up_sigma_diff( 
    const DepthInfo& o,
    Sigma sigma_diff, Sigma precision );

}
}

#endif
