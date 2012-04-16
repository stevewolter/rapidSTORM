#ifndef DSTORM_THREED_INFO_POLYNOMIAL3D_H
#define DSTORM_THREED_INFO_POLYNOMIAL3D_H

#include <dStorm/Direction.h>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/variant/variant.hpp>
#include <boost/optional/optional.hpp>
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
    static const int Order = polynomial_3d::Order, PrimaryTerm = 2, MinTerm = 1;
    typedef ZPosition FocalDepth;
    typedef boost::units::quantity< boost::units::si::length > WidthSlope;
    typedef Eigen::Array< Sigma, 2,1, Eigen::DontAlign > Sigmas;
    typedef Eigen::Array< FocalDepth, 2,1, Eigen::DontAlign > FocalPlanes;
    typedef Eigen::Array< FocalDepth, 2,1, Eigen::DontAlign > ZLimit;
private:
    Sigmas sigmas_;
    boost::optional< FocalPlanes > z_position;
    boost::optional< ZLimit > z_limit_;
    Eigen::Matrix< FocalDepth, 2, Order, Eigen::DontAlign > widening;

    std::string config_name_() const { return "Polynomial3D"; }
    Sigma get_sigma_( Direction dir, ZPosition z ) const;
    SigmaDerivative get_sigma_deriv_( Direction dir, ZPosition z ) const;
    ZRange z_range_() const;
    ZPosition equifocal_plane_() const;
    std::ostream& print_( std::ostream& ) const;
    bool provides_3d_info_() const { return true; }

    double sigma_scaling_factor( Direction, ZPosition ) const;
    FocalDepth lowest_z() const { return (*z_position - *z_limit_).minCoeff(); }
    FocalDepth highest_z() const { return (*z_position + *z_limit_).maxCoeff(); }

public:
    void set_base_width( Direction d, Sigma s ) { sigmas_[d] = s; }
    Sigma get_base_width( Direction d ) { return sigmas_[d]; }

    void set_prefactor( Direction, int term, FocalDepth focal_depth, double prefactor );
    double get_prefactor( Direction, int term ) const;
    FocalDepth get_focal_depth( Direction ) const;

    WidthSlope get_slope( Direction, int term ) const;
    void set_slope( Direction, int term, WidthSlope );

    const boost::optional< FocalPlanes > focal_planes() const { return z_position; }
    boost::optional< FocalPlanes >& focal_planes() { return z_position; }

    Eigen::Matrix< FocalDepth, Direction_2D, 1 > get_focal_depth() const;
    Eigen::Matrix< double, Direction_2D, Order > get_prefactors() const;

    const boost::optional<ZLimit>& z_limit() const { return z_limit_; }
    void set_z_limit( const ZLimit& z ) { z_limit_ = z; }

};

}
}

#endif
