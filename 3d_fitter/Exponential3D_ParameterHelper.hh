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
    const int x_low, const int y_low, const int z_layer
) {
    Base::extract( v, c );
    Eigen::Matrix<double,Kernels,1> _z, layer_z_distance;
    Eigen::Matrix<double,Kernels,2> z, z0, s0, delta_z, zn, sigmas, layer_delta;

    Base::template extract_param<MeanZ>(v, c, _z);
    Base::template extract_param<LayerDistance>(v, c, layer_z_distance);
    extract_param_xy<BestSigmaX,BestSigmaY>( v, c, s0 );
    extract_param_xy<DeltaSigmaX,DeltaSigmaY>( v, c, delta_z );
    extract_param_xy<ZAtBestSigmaX,ZAtBestSigmaY>( v, c, z0 );
    extract_param_xy<LayerShiftX,LayerShiftY>( v, c, layer_delta );

    for (int i = 0; i < z.cols(); ++i) {
        z.col(i) = _z;
        z0.col(i) += layer_z_distance * z_layer;
    }
    if ( Widening == Zhuang ) {
        delta_z = delta_z.cwise().sqrt();
    }

    this->translation = z_layer * layer_delta.row(0).transpose();

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
