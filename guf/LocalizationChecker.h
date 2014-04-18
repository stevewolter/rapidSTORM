#include "engine/JobInfo_decl.h"
#include <boost/icl/interval.hpp>
#include <boost/icl/interval_set.hpp>
#include "guf/Config_decl.h"
#include "gaussian_psf/fwd.h"
#include "guf/Spot.h"
#include "types/samplepos.h"

namespace dStorm {
namespace guf {

class MultiKernelModelStack;

/** Functor checking whether a fitted position is a valid localization. */
class LocalizationChecker {
    typedef boost::icl::interval_set< double > AllowedZPositions;

    const dStorm::engine::JobInfo& info;
    const double theta_dist_sq;
    AllowedZPositions allowed_z_positions;

    template <int Dim>
    bool check_kernel_dimension( const gaussian_psf::BaseExpression&, const guf::Spot&, int plane ) const;
    bool check_kernel( const gaussian_psf::BaseExpression&, const guf::Spot&, int plane ) const;

  public:
    LocalizationChecker( const Config&, const dStorm::engine::JobInfo& );
    bool operator()( const MultiKernelModelStack&, const guf::Spot& ) const;
};

}
}
