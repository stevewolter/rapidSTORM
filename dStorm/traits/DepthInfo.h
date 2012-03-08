#ifndef DSTORM_TRAITS_DEPTHINFO_H
#define DSTORM_TRAITS_DEPTHINFO_H

#include <dStorm/Direction.h>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/variant/variant.hpp>
#include <Eigen/Core>
#include <dStorm/polynomial_3d.h>

namespace dStorm {
namespace traits {

struct No3D {};

class Polynomial3D {
public:
    static const int Order = polynomial_3d::Order, PrimaryTerm = 2, MinTerm = 1;
    typedef boost::units::quantity< boost::units::si::length > FocalDepth;
    typedef boost::units::quantity< boost::units::si::length > WidthSlope;
private:
    Eigen::Matrix< FocalDepth, 2, Order, Eigen::DontAlign > widening;

public:
    void set_prefactor( Direction, int term, FocalDepth focal_depth, double prefactor );
    double get_prefactor( Direction, int term ) const;
    FocalDepth get_focal_depth( Direction ) const;

    WidthSlope get_slope( Direction, int term ) const;
    void set_slope( Direction, int term, WidthSlope );

    Eigen::Matrix< FocalDepth, Direction_2D, 1 > get_focal_depth() const;
    Eigen::Matrix< double, Direction_2D, Order > get_prefactors() const;
};

typedef boost::variant< traits::Polynomial3D, traits::No3D > DepthInfo;

}
}

#endif
