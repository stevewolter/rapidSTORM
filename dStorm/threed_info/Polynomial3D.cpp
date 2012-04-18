#include "DepthInfo.h"
#include <boost/units/Eigen/Core>
#include <boost/units/Eigen/Array>
#include <boost/units/cmath.hpp>
#include <boost/units/io.hpp>
#include <dStorm/threed_info/Polynomial3D.h>

namespace dStorm {
namespace threed_info {

using namespace boost::units;

void Polynomial3D::set_prefactor( Direction dir, int term, FocalDepth focal_depth, double prefactor )
{
    assert( term >= MinTerm && term <= Order );
    if ( prefactor < 1E-12 )
        widening( dir, term-1 ) = 1E30 * si::meter;
    else
        widening( dir, term-1 ) = focal_depth * float(pow( prefactor, -1.0 / term ));
}

double Polynomial3D::get_prefactor( Direction dir, int term ) const
{
    assert( term >= MinTerm && term <= Order );
    return pow( get_focal_depth(dir) / widening( dir, term-1 ), term );
}

Polynomial3D::FocalDepth Polynomial3D::get_focal_depth( Direction dir ) const {
    return widening( dir, PrimaryTerm-1 );
}

Polynomial3D::WidthSlope Polynomial3D::get_slope( Direction dir, int term ) const
{
    assert( term >= MinTerm && term <= Order );
    return widening( dir, term-1 );
}

void Polynomial3D::set_slope( Direction dir, int term, WidthSlope s )
{
    assert( term >= MinTerm && term <= Order );
    widening( dir, term-1 ) = s;
}

Eigen::Matrix< Polynomial3D::FocalDepth, Direction_2D, 1 > Polynomial3D::get_focal_depth() const
{
    Eigen::Matrix< FocalDepth, Direction_2D, 1 > rv;
    for (Direction dir = Direction_X; dir != Direction_2D; ++dir)
        rv[dir] = get_focal_depth(dir);
    return rv;
}

Eigen::Matrix< double, Direction_2D, Polynomial3D::Order > Polynomial3D::get_prefactors() const
{
    Eigen::Matrix< double, Direction_2D, Order > rv;
    for (Direction dir = Direction_X; dir != Direction_2D; ++dir)
        for (int term = MinTerm; term <= Order; ++term)
        rv(dir,term-MinTerm) = get_prefactor( dir, term );
    return rv;
}

Sigma Polynomial3D::get_sigma_( Direction dir, ZPosition z ) const 
{
    return sigmas_[dir] * float(sqrt(sigma_scaling_factor(dir,z)));
}

double Polynomial3D::sigma_scaling_factor( Direction dir, ZPosition z ) const {
    ZPosition z_offset = (z - (*z_position)[dir]);
    double prefactor = 1;
    for (int i = MinTerm; i <= Order; ++i ) {
        prefactor += pow( z_offset / widening( i-1, dir ), i );
    }
    return prefactor;
}

SigmaDerivative Polynomial3D::get_sigma_deriv_( Direction dir, ZPosition z ) const {
    ZPosition z_offset = (z - (*z_position)[dir]);
    double prefactor_deriv = 0;
    for (int i = MinTerm; i <= Order; ++i ) {
        prefactor_deriv += 
                pow( z_offset / widening( i-1, dir ), i-1 ) * (sigmas_[dir] / widening( i-1, dir ));
    }
    return (prefactor_deriv / (2 * sqrt( sigma_scaling_factor(dir,z) )));
}

ZRange Polynomial3D::z_range_() const {
    ZRange rv;
    rv += ZInterval( lowest_z(), highest_z() );
    return rv;
}

ZPosition Polynomial3D::equifocal_plane_() const {
    return ( z_position->x() + z_position->y() ) / 2.0f;
}

std::ostream& Polynomial3D::print_( std::ostream& o ) const {
    o << "polynomial 3D with best-focused FWHMs " << sigmas_[0] << " and " << sigmas_[1]
      << ", X focus depths " ;
    for (int j = threed_info::Polynomial3D::MinTerm; j <= threed_info::Polynomial3D::Order; ++j)
        o << 1.0 / get_slope(Direction_X, j) << " ";
    o << " and Y focus depth " ;
    for (int j = threed_info::Polynomial3D::MinTerm; j <= threed_info::Polynomial3D::Order; ++j)
        o << 1.0 / get_slope(Direction_Y, j) << " ";
    return o << " and focal planes " << focal_planes()->transpose();
}

}
}
