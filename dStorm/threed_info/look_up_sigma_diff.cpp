#include <boost/units/Eigen/Core>
#include "look_up_sigma_diff.h"
#include "equifocal_plane.h"
#include <dStorm/traits/optics.h>
#include <boost/variant/apply_visitor.hpp>
#include <dStorm/threed_info/Spline.h>

namespace dStorm {
namespace traits {

using namespace boost::units;

class look_up_sigma_diff_visitor
: public boost::static_visitor< quantity<si::length,float> >
{
    quantity<si::length> sigma_diff;
public:
    look_up_sigma_diff_visitor( quantity<si::length> sigma_diff ) 
        : sigma_diff(sigma_diff) {}
    quantity<si::length,float> operator()( const traits::No3D& ) const
        { return 0 * si::meter; }
    quantity<si::length,float> operator()( const traits::Polynomial3D& p ) const {
        return equifocal_plane(p);
    }
    quantity<si::length,float> operator()( const traits::Spline3D& s ) const
    { 
        return quantity<si::length,float>(s.get_spline()->look_up_sigma_diff( sigma_diff, 1E-8 * si::meter )
            .get_value_or( s.equifocal_plane() ));
    }
};

quantity<si::length,float> look_up_sigma_diff( const DepthInfo& o, quantity<si::length> sigma_diff ) {
    return boost::apply_visitor( look_up_sigma_diff_visitor(sigma_diff), o );
}

}
}
