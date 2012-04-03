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

    typedef boost::optional< quantity<si::length> > ZPosition;
    ZPosition look_up_sigma_diff( quantity<si::length> sigma_x, quantity<si::length> sigma_y,
                                  quantity<si::length> precision ) const;

private:
    const int N;
    friend class SplineFactory;
    struct Point {
        quantity<si::length> z;
        quantity<si::length> sigma[Direction_2D];
    };
    boost::shared_array<const double> zs, sigmas[Direction_2D];
    boost::shared_ptr< const gsl_interp > splines[Direction_2D];
};

class SplineFactory {
public:
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
