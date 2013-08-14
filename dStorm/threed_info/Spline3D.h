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

#include <Eigen/Core>

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
    const int N;
    friend class SplineFactory;
    struct Point {
        ZPosition z;
        Sigma sigma;
    };
    const std::vector<Point> points;
    const ZPosition h;
    Eigen::MatrixXd coeffs;
    ZPosition equifocal_plane__;

    std::string config_name_() const { return "Spline3D"; }
    Sigma get_sigma_( ZPosition z ) const;
    SigmaDerivative get_sigma_deriv_( ZPosition z ) const;
    ZRange z_range_() const;
    ZPosition equifocal_plane_() const { return equifocal_plane__; }
    std::ostream& print_( std::ostream& o ) const 
        { return o << "spline 3D information"; }
    bool provides_3d_info_() const { return true; }
};

class SplineFactory {
public:
    SplineFactory( const std::string& file, Direction dir );

    static SplineFactory Mock( Direction d );

private:
    SplineFactory() {}
    friend class Spline3D;
    typedef std::vector< Spline3D::Point > Points;
    Points points;
};

}
}

#endif
