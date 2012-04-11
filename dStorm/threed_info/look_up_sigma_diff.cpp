#include <boost/units/Eigen/Core>
#include "look_up_sigma_diff.h"
#include "equifocal_plane.h"
#include <boost/variant/apply_visitor.hpp>
#include <dStorm/threed_info/Spline3D.h>
#include <boost/units/cmath.hpp>
#include <boost/units/io.hpp>
#include <boost/lexical_cast.hpp>
#include "depth_range.h"
#include "get_sigma.h"

namespace dStorm {
namespace threed_info {

using namespace boost::units;

Sigma SigmaDiffLookup::get_sigma_diff( ZPosition z ) const {
    return get_sigma( a, a_dim, z ) - get_sigma( b, b_dim, z );
}

ZRange SigmaDiffLookup::get_z_range() const { return threed_info::get_z_range(a) & threed_info::get_z_range(b); }

boost::optional<ZPosition> SigmaDiffLookup::look_up_sigma_diff( Sigma sigma_diff, Sigma precision ) const
{
    ZRange common_range = get_z_range();
    ZPosition lower_bound = lower( common_range ),
              upper_bound = upper( common_range );

    /* Switch bounds if the gradient is negative, so we can assume in 
    * the rest of the search that lower_bound is at the Y-larger-X end
    * (not necessarily the low-Z end). */
    Sigma lower_sigma, upper_sigma, test_sigma;
    lower_sigma = get_sigma_diff( lower_bound );
    upper_sigma = get_sigma_diff( upper_bound );
    if ( lower_sigma > upper_sigma ) {
        std::swap( lower_bound, upper_bound );
        std::swap( lower_sigma, upper_sigma );
    }

    if ( lower_sigma > sigma_diff || upper_sigma < sigma_diff )
        return boost::optional<ZPosition>();
    else {
        while ( abs( upper_bound - lower_bound ) > precision ) {
            ZPosition test_x = (lower_bound + upper_bound) / 2.0f;
            test_sigma = get_sigma_diff( test_x );
            if ( test_sigma > sigma_diff )
                upper_bound = test_x;
            else
                lower_bound = test_x;
        }

        return (upper_bound + lower_bound) / 2.0f;
    }
}

boost::optional<ZPosition> look_up_sigma_diff( const DepthInfo& o, Sigma sigma_diff, Sigma precision ) {
    return SigmaDiffLookup(o).look_up_sigma_diff( sigma_diff, precision );
}

}
}
