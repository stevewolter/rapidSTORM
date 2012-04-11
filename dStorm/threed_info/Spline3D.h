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

namespace dStorm {
namespace threed_info {

using boost::units::quantity;
namespace si = boost::units::si;

class SplineFactory;

class Spline3D {
public:
    Spline3D( const SplineFactory& );

    Sigma get_sigma( Direction dir, ZPosition z ) const;
    Sigma get_sigma_diff( ZPosition z ) const;
    SigmaDerivative get_sigma_deriv( Direction dir, ZPosition z ) const;
    ZRange z_range() const;
    quantity<si::length> equifocal_plane() const
        { return equifocal_plane_; }

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
    quantity<si::length> equifocal_plane_;
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
