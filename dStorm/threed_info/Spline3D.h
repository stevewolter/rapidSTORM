#ifndef DSTORM_THREED_INFO_SPLINE_H
#define DSTORM_THREED_INFO_SPLINE_H

#include <vector>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/shared_array.hpp>
#include <boost/optional/optional.hpp>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/length.hpp>
#include <dStorm/Direction.h>
#include <gsl/gsl_interp.h>

#include <dStorm/Localization_decl.h>
#include "types.h"
#include "DepthInfo.h"

namespace dStorm {
namespace threed_info {

using boost::units::quantity;
namespace si = boost::units::si;

class SplineFactory;

class Spline3D : public DepthInfo {
public:
    Spline3D( const SplineFactory& );

private:
    int N;
    friend class SplineFactory;
    struct Point {
        quantity<si::length> z;
        quantity<si::length> sigma[Direction_2D];
    };
    double border_coeffs[2][2][4];
    boost::shared_array<const double> zs;
    boost::shared_array<const double> sigmas[Direction_2D];
    boost::shared_ptr< const gsl_interp > splines[Direction_2D];
    ZPosition equifocal_plane__;

    std::string config_name_() const { return "Spline3D"; }
    Sigma get_sigma_( Direction dir, ZPosition z ) const;
    SigmaDerivative get_sigma_deriv_( Direction dir, ZPosition z ) const;
    ZRange z_range_() const;
    ZPosition equifocal_plane_() const { return equifocal_plane__; }
    std::ostream& print_( std::ostream& o ) const 
        { return o << "spline 3D information"; }
    bool provides_3d_info_() const { return true; }
};

class SplineFactory {
public:
    SplineFactory() {}
    SplineFactory( const std::string& file );

    static SplineFactory Mock();

    void add_point( 
        quantity<si::length> z_position,
        quantity<si::length> sigma_x,
        quantity<si::length> sigma_y );

private:
    friend class Spline3D;
    typedef std::vector< Spline3D::Point > Points;
    Points points;
};

}
}

#endif
