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
    Base::template extract_param<BestSigmaX>( v, c, v0x );
    Base::template extract_param<BestSigmaY>( v, c, v0y );
    Base::template extract_param<DeltaSigmaX>( v, c, this->dzx );
    Base::template extract_param<DeltaSigmaY>( v, c, this->dzy );
    Base::template extract_param<ZAtBestSigmaX>( v, c, z0x );
    Base::template extract_param<ZAtBestSigmaY>( v, c, z0y );
    Base::template extract_param<MeanZ>( v, c, z );

    zdx = (z - z0x).cwise() * this->dzx; zdy = (z - z0y).cwise() * this->dzy;
    if ( Special::Use_Holtzer_PSF ) {
        this->sx = (zdx.cwise().square() + v0x.cwise().square()).cwise().sqrt();
        this->sy = (zdy.cwise().square() + v0y.cwise().square()).cwise().sqrt();
    } else {
        this->sx = zdx.cwise().square() + v0x;
        this->sy = zdy.cwise().square() + v0y;
    }
    zdx.cwise() *= dzx; zdy.cwise() *= dzy;

    if ( ! Base::check(x_low, y_low) )
        return false;

    Base::precompute(x_low, y_low);
    return true;
};

}
}

#endif
