#include "gaussian_psf/DepthInfo3D.h"
#include "nonlinfit/InvalidPositionError.h"

namespace dStorm {
namespace gaussian_psf {

void DepthInfo3D::set_spline( DepthInfo sx, DepthInfo sy )
{ 
    assert( sx.get() && sx->provides_3d_info() );
    assert( sy.get() && sy->provides_3d_info() );
    spline[0] = sx;
    spline[1] = sy; 
}

template <typename Number>
Eigen::Array<Number,2,1> Parameters<Number,DepthInfo3D>::compute_sigma_() 
{
    threed_info::ZPosition z( (*expr)( MeanZ() ) );
        
    Eigen::Array<Number,2,1> rv;
    for (Direction i = Direction_First; i != Direction_2D; ++i)
    {
        rv[i] = expr->spline[i]->get_sigma(z);
    }
    return rv;
}

template <typename Number>
void Parameters<Number,DepthInfo3D>::compute_prefactors_() {
    threed_info::ZPosition z( (*expr)( MeanZ() ) );
    this->sigma_deriv = this->sigmaI;
    for (Direction i = Direction_First; i != Direction_2D; ++i)
    {
        z_deriv_prefactor[i] = - this->sigmaI[i] * 
            expr->spline[i]->get_sigma_deriv( z );
    }
}

Eigen::Vector2d DepthInfo3D::get_sigma() const
{
    Parameters<double,DepthInfo3D> evaluator(*this);
    return evaluator.compute_sigma();
}


template class Parameters< double, DepthInfo3D >;
template class Parameters< float, DepthInfo3D >;

}
}
