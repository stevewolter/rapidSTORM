#ifndef DSTORM_TRAITS_NO3D_H
#define DSTORM_TRAITS_NO3D_H

#include "DepthInfo.h"

namespace dStorm {
namespace threed_info {

struct No3D : public DepthInfo {
    Sigma sigma;
    std::string config_name_() const { return "No3D"; }
    Sigma get_sigma_( ZPosition z ) const { return sigma; }
    SigmaDerivative get_sigma_deriv_( ZPosition ) const 
        { throw std::logic_error("Attempted to get dSigma/dZ for no-3D model"); }
    ZRange z_range_() const { return ZRange(); }
    std::ostream& print_( std::ostream& o ) const {
            return o << "no 3D information with PSF width " << sigma * 2.35f;
    }
    bool provides_3d_info_() const { return false; }
};

}
}

#endif
