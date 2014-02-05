#ifndef DSTORM_THREED_INFO_LOOK_UP_SIGMA_DIFF_H
#define DSTORM_THREED_INFO_LOOK_UP_SIGMA_DIFF_H

#include "dStorm/threed_info/DepthInfo.h"
#include <boost/icl/continuous_interval.hpp>
#include <dStorm/types/samplepos.h>
#include <dStorm/Direction.h>
#include <boost/optional/optional.hpp>
#include "dStorm/threed_info/types.h"
#include <vector>

namespace dStorm {
namespace threed_info {

struct SigmaDiffLookup {
    const DepthInfo &a, &b;
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
    SigmaDiffLookup( const DepthInfo& minuend, const DepthInfo& subtrahend, ZPosition precision )
        : a( minuend ), b( subtrahend ), precision(precision) { init(); }

    Sigma get_sigma_diff( ZPosition ) const;
    ZPosition operator()( Sigma a, Sigma b ) const;
    ZRange get_z_range() const;
};

}
}

#endif
