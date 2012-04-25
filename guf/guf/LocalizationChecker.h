#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/length.hpp>
#include <dStorm/engine/JobInfo_decl.h>
#include <dStorm/units/icl.h>
#include <boost/icl/interval.hpp>
#include <boost/icl/interval_set.hpp>
#include "Config_decl.h"
#include "gaussian_psf/fwd.h"
#include "Spot.h"
#include <dStorm/types/samplepos.h>

namespace dStorm {
namespace guf {

class MultiKernelModelStack;

/** Functor checking whether a fitted position is a valid localization. */
class LocalizationChecker {
    typedef boost::icl::interval_set< samplepos::Scalar > AllowedZPositions;

    const dStorm::engine::JobInfo& info;
    const boost::units::quantity<boost::units::si::length> theta_dist;
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
