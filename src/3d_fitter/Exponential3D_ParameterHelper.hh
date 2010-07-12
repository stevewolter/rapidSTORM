#ifndef LIBFITPP_EXPONENTIAL3D_PARAMETERHELPER_H
#define LIBFITPP_EXPONENTIAL3D_PARAMETERHELPER_H

#include "Exponential3D_impl.hh"
#include <Eigen/Array>

namespace fitpp {
namespace Exponential3D {

template <int Kernels, int Widening, int W, int H>
bool ParameterHelper< Kernels, Widening, W, H >::prepare(
    const typename MySpecialization::Space::Variables& v,
    const typename MySpecialization::Space::Constants& c,
    const int x_low, const int y_low
) {
    Base::extract( v, c );
    Eigen::Matrix<double,Kernels,1> _z;
    Eigen::Matrix<double,Kernels,2> z, z0, s0, delta_z, zn, sigmas;

    Base::template extract_param<MeanZ>(v, c, _z);
    z.col(0) = _z;
    z.col(1) = _z;
    extract_param_xy<BestSigmaX,BestSigmaY>( v, c, s0 );
    extract_param_xy<DeltaSigmaX,DeltaSigmaY>( v, c, delta_z );
    extract_param_xy<ZAtBestSigmaX,ZAtBestSigmaY>( v, c, z0 );
    if ( Widening == Zhuang ) {
        delta_z = delta_z.cwise().sqrt();
    }

    zn = (z - z0).cwise() * delta_z;
    if ( Widening == Holtzer ) {
        sigmas = (zn.cwise().square() + s0.cwise().square()).cwise().sqrt();
        this->z_deriv_prefactor = (zn.cwise() * 
            (delta_z.cwise() * sigmas.cwise().inverse().cwise().square()));
    } else if ( Widening == Zhuang ) {
        sigmas = zn.cwise().square() + s0;
        this->z_deriv_prefactor = 2 * (zn.cwise() * 
            (delta_z.cwise() * sigmas.cwise().inverse()));
    } else {
        /* Invalid Widening parameter */
        return false;
    }
    this->sx = sigmas.col(0);
    this->sy = sigmas.col(1);

    if ( ! Base::check(x_low, y_low) )
        return false;

    Base::precompute(x_low, y_low);
    return true;
};

}
}

#endif
