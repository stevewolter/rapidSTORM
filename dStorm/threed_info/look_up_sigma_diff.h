#ifndef DSTORM_THREED_INFO_LOOK_UP_SIGMA_DIFF_H
#define DSTORM_THREED_INFO_LOOK_UP_SIGMA_DIFF_H

#include "DepthInfo.h"
#include <boost/icl/continuous_interval.hpp>
#include <dStorm/types/samplepos.h>
#include <dStorm/Direction.h>
#include <boost/optional/optional.hpp>
#include "types.h"
#include <vector>

namespace dStorm {
namespace threed_info {

struct SigmaDiffLookup {
    const DepthInfo &a, &b;
    const Direction a_dim, b_dim;
    const ZPosition precision;

    struct Diff {
        ZPosition z;
        Sigma diff;
        static bool smaller_sigma_diff( const Diff& a, const Diff& b )
            { return a.diff < b.diff; }
    };
    std::vector<Diff> diffs;
    void init();
    void crop_to_longest_monotonic_sequence();

public:
    SigmaDiffLookup( 
        const DepthInfo& minuend,
        Direction minuend_dimension,
        const DepthInfo& subtrahend,
        Direction subtrahend_dimension,
        ZPosition precision );

    SigmaDiffLookup( const DepthInfo& o, ZPosition precision )
        : a( o ), b( o ), a_dim( Direction_X ), b_dim( Direction_Y ), precision(precision) { init(); }

    Sigma get_sigma_diff( ZPosition ) const;
    ZPosition operator()( Sigma a, Sigma b ) const;
    ZRange get_z_range() const;
};

}
}

#endif
