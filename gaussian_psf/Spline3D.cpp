#include "Spline3D.h"
#include "gaussian_psf/check_evaluator.hpp"

namespace dStorm {
namespace gaussian_psf {

void Spline3D::set_spline( DepthInfo sx, DepthInfo sy )
{ 
    assert( sx.get() && sx->provides_3d_info() );
    assert( sy.get() && sy->provides_3d_info() );
    spline[0] = sx;
    spline[1] = sy; 
}

template <typename Number>
boost::optional< Eigen::Array<Number,2,1> > Parameters<Number,Spline3D>::compute_sigma_() 
{
    threed_info::ZPosition z( (*expr)( MeanZ() ) );
        
    Eigen::Array<Number,2,1> rv;
    for (Direction i = Direction_First; i != Direction_2D; ++i)
    {
        threed_info::Sigma s = expr->spline[i]->get_sigma(z);
        rv[i] = quantity< BestSigma<0>::Unit >( s ).value();
    }
    if ( (rv.array() <= 0).any() )
        return boost::optional< Eigen::Array<Number,2,1> >();
    else
        return rv;
}

template <typename Number>
void Parameters<Number,Spline3D>::compute_prefactors_() {
    threed_info::ZPosition z( (*expr)( MeanZ() ) );
    this->sigma_deriv = this->sigmaI;
    for (Direction i = Direction_First; i != Direction_2D; ++i)
    {
        z_deriv_prefactor[i] = - this->sigmaI[i] * 
            expr->spline[i]->get_sigma_deriv( z );
    }
}

Eigen::Matrix< quantity<LengthUnit>, 2, 1 > Spline3D::get_sigma() const
{
    Parameters<double,Spline3D> evaluator(*this);
    return boost::units::from_value< LengthUnit >( evaluator.compute_sigma() );
}


template class Parameters< double, Spline3D >;
template class Parameters< float, Spline3D >;

using namespace nonlinfit::plane;

template boost::unit_test::test_suite* check_evaluator<Spline3D>( const char* name );

}
}
