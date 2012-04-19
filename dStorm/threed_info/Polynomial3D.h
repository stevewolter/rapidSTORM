#ifndef DSTORM_THREED_INFO_POLYNOMIAL3D_H
#define DSTORM_THREED_INFO_POLYNOMIAL3D_H

#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/variant/variant.hpp>
#include <Eigen/Core>
#include <dStorm/polynomial_3d.h>
#include <dStorm/threed_info/fwd.h>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <dStorm/threed_info/DepthInfo.h>
#include "types.h"

namespace dStorm {
namespace threed_info {

class Polynomial3D : public DepthInfo {
public:
    Polynomial3D();
    static const int Order = polynomial_3d::Order, PrimaryTerm = 2, MinTerm = 1;
    typedef boost::units::quantity< boost::units::si::length > WidthSlope;
private:
    typedef Eigen::Matrix< WidthSlope, Order+1, 1, Eigen::DontAlign > Widening;

    Sigma sigma_;
    ZPosition z_position, z_limit_;
    Widening widening;

    std::string config_name_() const { return "Polynomial3D"; }
    Sigma get_sigma_( ZPosition z ) const;
    SigmaDerivative get_sigma_deriv_( ZPosition z ) const;
    ZRange z_range_() const;
    std::ostream& print_( std::ostream& ) const;
    bool provides_3d_info_() const { return true; }

    double sigma_scaling_factor( ZPosition ) const;

public:
    void set_base_width( Sigma s ) { sigma_ = s; }
    const Sigma get_base_width( ) const { return sigma_; }

    WidthSlope get_slope( int term ) const;
    void set_slope( int term, WidthSlope );

    const ZPosition focal_plane() const { return z_position; }
    void set_focal_plane( ZPosition a ) { z_position = a; }

    const ZPosition z_limit() const { return z_limit_; }
    void set_z_limit( const ZPosition& z ) { z_limit_ = z; }

};

}
}

#endif
