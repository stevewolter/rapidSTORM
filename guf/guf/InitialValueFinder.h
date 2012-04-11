#ifndef DSTORM_FITTER_GUF_INITIAL_VALUE_FINDER_H
#define DSTORM_FITTER_GUF_INITIAL_VALUE_FINDER_H

#include "FitAnalysis.h"
#include "Config.h"
#include <dStorm/engine/Input_decl.h>
#include <dStorm/engine/JobInfo_decl.h>
#include "Statistics.h"
#include <vector>
#include <dStorm/Direction.h>
#include <dStorm/threed_info/look_up_sigma_diff.h>

namespace dStorm {
namespace guf {

struct InitialValueFinder {
    typedef void result_type;

    const dStorm::engine::JobInfo& info;
    const bool disjoint_amplitudes;
    struct SigmaDiff {
        int minuend_plane, subtrahend_plane;
        Direction minuend_dir, subtrahend_dir;

        threed_info::SigmaDiffLookup lookup( const dStorm::engine::JobInfo& info ) const;
    };
    boost::optional< SigmaDiff > most_discriminating_diff;
    float correlation( const SigmaDiff& ) const;

    struct PlaneEstimate;
    std::vector<PlaneEstimate> estimate_bg_and_amp( const Spot& spot, const Statistics<3> & ) const;
    void join_amp_estimates( std::vector<PlaneEstimate>& v ) const;
    void estimate_z( const Statistics<3>&, std::vector<PlaneEstimate>& ) const;

    class set_parameter;
    template <typename Parameter>
    inline void operator()( Parameter, FittedPlane&, const Spot&, const PlaneEstimate& ) const;

  public:
    InitialValueFinder( const Config& config, const dStorm::engine::JobInfo& info);

    void operator()( FitPosition& position, const Spot&, const Statistics<3>& ) const;
};

}
}

#endif
