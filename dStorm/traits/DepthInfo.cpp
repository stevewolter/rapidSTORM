#include "DepthInfo.h"
#include <dStorm/threed_info/Spline.h>

namespace dStorm {
namespace traits {

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

boost::units::quantity< boost::units::si::length > Spline3D::equifocal_plane() const
    { return spline->equifocal_plane(); }

}
}
