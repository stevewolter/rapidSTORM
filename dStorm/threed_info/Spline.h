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

namespace dStorm {
namespace threed_info {

using boost::units::quantity;
namespace si = boost::units::si;

class SplineFactory;

class Spline {
public:
    Spline( const SplineFactory& );

    typedef boost::optional< quantity<si::length> > Sigma;
    Sigma get_sigma( Direction dir, quantity<si::length> z ) const;
    Sigma get_sigma_diff( quantity<si::length> z ) const;

    typedef boost::optional< double > SigmaDerivative;
    SigmaDerivative get_sigma_deriv( Direction dir, quantity<si::length> z ) const;

    typedef boost::optional< quantity<si::length> > ZPosition;
    ZPosition look_up_sigma_diff( quantity<si::length> sigma_x, quantity<si::length> sigma_y,
                                  quantity<si::length> precision ) const;
    ZPosition look_up_sigma_diff( const Localization&, quantity<si::length> precision ) const;

    typedef std::pair< ZPosition, ZPosition > Range;
    Range get_range() const;

    quantity<si::length> equifocal_plane() const
        { return equifocal_plane_; }

private:
    const int N;
    friend class SplineFactory;
    struct Point {
        quantity<si::length> z;
        quantity<si::length> sigma[Direction_2D];
    };
    boost::shared_array<const double> zs, sigmas[Direction_2D];
    boost::shared_ptr< const gsl_interp > splines[Direction_2D];
    quantity<si::length> equifocal_plane_;

    ZPosition look_up_sigma_diff( quantity<si::length> sigma_diff, quantity<si::length> precision ) const;
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
    friend class Spline;
    typedef std::vector< Spline::Point > Points;
    Points points;
};

}
}

#endif
