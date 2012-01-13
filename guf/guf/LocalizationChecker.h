#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/length.hpp>
#include <dStorm/engine/JobInfo_decl.h>
#include <boost/icl/interval.hpp>
#include <boost/icl/interval_set.hpp>
#include "Config_decl.h"
#include "guf/psf/fwd.h"
#include "Spot.h"
#include "FitAnalysis.h"

namespace dStorm {
namespace guf {

/** Functor checking whether a fitted position is a valid localization. */
class LocalizationChecker {
    typedef boost::icl::interval_set< boost::units::quantity<
        boost::units::si::length > > AllowedZPositions;

    const dStorm::engine::JobInfo& info;
    const boost::units::quantity<boost::units::si::length> theta_dist;
    AllowedZPositions allowed_z_positions;

    template <int Dim>
    bool check_kernel_dimension( const PSF::BaseExpression&, const guf::Spot& ) const;
    bool check_kernel( const PSF::BaseExpression&, const guf::Spot& ) const;

  public:
    LocalizationChecker( const Config&, const dStorm::engine::JobInfo& );
    bool operator()( const FitPosition&, const guf::Spot& ) const;
};

}
}
