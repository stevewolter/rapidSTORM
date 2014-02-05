#ifndef DSTORM_TRAITS_NO3D_H
#define DSTORM_TRAITS_NO3D_H

#include "dStorm/threed_info/DepthInfo.h"

namespace dStorm {
namespace threed_info {

struct No3D : public DepthInfo {
    Sigma sigma;
    std::string config_name_() const { return "No3D"; }
    Sigma get_sigma_( ZPosition z ) const { return sigma; }
    SigmaDerivative get_sigma_deriv_( ZPosition ) const ;
    ZRange z_range_() const { return ZRange(); }
    std::ostream& print_( std::ostream& o ) const; 
    bool provides_3d_info_() const { return false; }
};

}
}

#endif
