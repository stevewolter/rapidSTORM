#ifndef DSTORM_TRAITS_DEPTHINFO_H
#define DSTORM_TRAITS_DEPTHINFO_H

#include <dStorm/Direction.h>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/variant/variant.hpp>
#include <boost/optional/optional.hpp>
#include <Eigen/Core>
#include <dStorm/polynomial_3d.h>
#include <dStorm/threed_info/fwd.h>
#include <boost/smart_ptr/shared_ptr.hpp>

namespace dStorm {
namespace traits {

struct No3D {};

class Polynomial3D {
public:
    static const int Order = polynomial_3d::Order, PrimaryTerm = 2, MinTerm = 1;
    typedef boost::units::quantity< boost::units::si::length > FocalDepth;
    typedef boost::units::quantity< boost::units::si::length > WidthSlope;
    typedef Eigen::Array< FocalDepth, 2,1, Eigen::DontAlign > FocalPlanes;
private:
    boost::optional< FocalPlanes > z_position;
    Eigen::Matrix< FocalDepth, 2, Order, Eigen::DontAlign > widening;

public:
    void set_prefactor( Direction, int term, FocalDepth focal_depth, double prefactor );
    double get_prefactor( Direction, int term ) const;
    FocalDepth get_focal_depth( Direction ) const;

    WidthSlope get_slope( Direction, int term ) const;
    void set_slope( Direction, int term, WidthSlope );

    const boost::optional< FocalPlanes > focal_planes() const { return z_position; }
    boost::optional< FocalPlanes >& focal_planes() { return z_position; }

    Eigen::Matrix< FocalDepth, Direction_2D, 1 > get_focal_depth() const;
    Eigen::Matrix< double, Direction_2D, Order > get_prefactors() const;
};

class Spline3D {
    boost::shared_ptr< const threed_info::Spline > spline;
  public:
    Spline3D ( boost::shared_ptr< const threed_info::Spline > s ) : spline(s) {}
    const boost::shared_ptr< const threed_info::Spline > get_spline() const
        { return spline; }
    boost::units::quantity< boost::units::si::length > equifocal_plane() const;
};

typedef boost::variant< traits::Polynomial3D, traits::No3D, traits::Spline3D > DepthInfo;

}
}

#endif
