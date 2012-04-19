#include "DepthInfo.h"
#include <boost/units/Eigen/Core>
#include <boost/units/Eigen/Array>
#include <boost/units/cmath.hpp>
#include <boost/units/io.hpp>
#include <dStorm/threed_info/Polynomial3D.h>

namespace dStorm {
namespace threed_info {

using namespace boost::units;

Polynomial3D::Polynomial3D() 
: sigma_(0.0 * si::meter),
  z_position( 0.0 * si::meter ),
  z_limit_( 0.0 * si::meter ),
  widening( Widening::Constant( 0.0 * si::meter ) )
{
}

Polynomial3D::WidthSlope Polynomial3D::get_slope( int term ) const
{
    assert( term >= MinTerm && term <= Order );
    return widening[ term ];
}

void Polynomial3D::set_slope( int term, WidthSlope s )
{
    assert( term >= MinTerm && term <= Order );
    widening[ term ] = s;
}

Sigma Polynomial3D::get_sigma_( ZPosition z ) const 
{
    return sigma_ * float(sqrt(sigma_scaling_factor(z)));
}

double Polynomial3D::sigma_scaling_factor( ZPosition z ) const {
    ZPosition z_offset = (z - z_position);
    double prefactor = 1;
    for (int i = MinTerm; i <= Order; ++i ) {
        prefactor += pow( z_offset / widening[ i ], i );
    }
    return prefactor;
}

SigmaDerivative Polynomial3D::get_sigma_deriv_( ZPosition z ) const {
    ZPosition z_offset = (z - z_position);
    double prefactor_deriv = 0;
    for (int i = MinTerm; i <= Order; ++i ) {
        prefactor_deriv += 
                pow( z_offset / widening[ i ], i-1 ) * (sigma_ / widening[ i ] );
    }
    return (prefactor_deriv / (2 * sqrt( sigma_scaling_factor(z) )));
}

ZRange Polynomial3D::z_range_() const {
    ZRange rv;
    rv += ZInterval( z_position - z_limit_, z_position + z_limit_ );
    return rv;
}

std::ostream& Polynomial3D::print_( std::ostream& o ) const {
    o << "polynomial 3D with best-focused FWHM " << sigma_ 
      << "and focus depths " ;
    for (int j = threed_info::Polynomial3D::MinTerm; j <= threed_info::Polynomial3D::Order; ++j)
        o << 1.0 / get_slope(j) << " ";
    return o << " and focal plane " << z_position;
}

}
}
