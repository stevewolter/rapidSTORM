#ifndef LIBFITPP_EXPONENTIAL3D_PARAMETERHELPER_H
#define LIBFITPP_EXPONENTIAL3D_PARAMETERHELPER_H

#include "Exponential3D_impl.hh"
#include <Eigen/Array>

namespace fitpp {
namespace Exponential3D {

template <class Special>
bool ParameterHelper<Special>::prepare(
    const typename Special::Space::Variables& v,
    const typename Special::Space::Constants& c,
    const int x_low, const int y_low
) {
    Base::extract( v, c );
    Eigen::Matrix<double,Kernels,1> z, z0x, z0y, v0x, v0y;
    Base::template extract_param<BestVarianceX>( v, c, v0x );
    Base::template extract_param<BestVarianceY>( v, c, v0y );
    Base::template extract_param<DeltaSigmaX>( v, c, this->dzx );
    Base::template extract_param<DeltaSigmaY>( v, c, this->dzy );
    Base::template extract_param<ZAtBestSigmaX>( v, c, z0x );
    Base::template extract_param<ZAtBestSigmaY>( v, c, z0y );
    Base::template extract_param<MeanZ>( v, c, z );

    zdx = (z - z0x).cwise() * this->dzx; zdy = (z - z0y).cwise() * this->dzy;
    this->sx = (zdx.cwise().square() + v0x).cwise().sqrt();
    this->sy = (zdy.cwise().square() + v0y).cwise().sqrt();
    zdx.cwise() *= dzx; zdy.cwise() *= dzy;

    if ( ! Base::check(x_low, y_low) )
        return false;

    Base::precompute(x_low, y_low);
    return true;
};

}
}

#endif
