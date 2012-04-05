#include "DepthInfo.h"
#include <dStorm/threed_info/Spline3D.h>
#include <boost/units/cmath.hpp>
#include <boost/units/io.hpp>

namespace dStorm {
namespace threed_info {

using namespace boost::units;

void Polynomial3D::set_prefactor( Direction dir, int term, FocalDepth focal_depth, double prefactor )
{
    assert( term >= MinTerm && term <= Order );
    if ( prefactor < 1E-12 )
        widening( dir, term-1 ) = 1E30 * si::meter;
    else
        widening( dir, term-1 ) = focal_depth * pow( prefactor, -1.0 / term );
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

boost::optional< Polynomial3D::Sigma > 
    Polynomial3D::get_sigma_diff( FocalDepth z ) const 
{
    if ( z < lowest_z() || z > highest_z() ) return boost::optional< Polynomial3D::Sigma >();
    double prefactor[2] = {1,1};
    for (int i = MinTerm; i <= Order; ++i ) {
        for ( Direction dir = Direction_First; dir != Direction_2D; ++dir ) {
            prefactor[dir] += 
                pow( (z - (*z_position)[dir]) / widening( i-1, dir ), i );
        }
    }
    Polynomial3D::Sigma diff = sigmas_.x() * sqrt(prefactor[0]) - sigmas_.y() * sqrt(prefactor[1]);
    return diff;
}

}
}
