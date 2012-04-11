#ifndef DSTORM_TRAITS_NO3D_H
#define DSTORM_TRAITS_NO3D_H

#include "DepthInfo.h"

namespace dStorm {
namespace threed_info {

struct No3D : public DepthInfo {
    Sigma sigma[2];
    std::string config_name_() const { return "No3D"; }
    Sigma get_sigma_( Direction dir, ZPosition z ) const { return sigma[dir]; }
    SigmaDerivative get_sigma_deriv_( Direction, ZPosition ) const { return 0; }
    ZRange z_range_() const { return ZRange(); }
    ZPosition equifocal_plane_() const { return 0 * si::meter; }
    std::ostream& print_( std::ostream& o ) const {
            return o << "no 3D information";
    }
    bool provides_3d_info_() const { return false; }
};

}
}

#endif
