#ifndef DSTORM_TRAITS_NO3D_H
#define DSTORM_TRAITS_NO3D_H

#include "dStorm/threed_info/DepthInfo.h"
#include <stdexcept>

namespace dStorm {
namespace threed_info {

class Lens3DConfig;

struct Lens3D : public DepthInfo {
    std::string config_name_() const { return "Lens3D"; }
    Sigma get_sigma_( ZPosition z ) const 
        { throw std::logic_error("Lens3D cannot be used for gaussian fitting"); }
    SigmaDerivative get_sigma_deriv_( ZPosition ) const
        { throw std::logic_error("Lens3D cannot be used for gaussian fitting"); }
    ZRange z_range_() const { return ZRange(); }
    std::ostream& print_( std::ostream& o ) const; 
    bool provides_3d_info_() const { return false; }
    ZPosition z_position_;
public:
    ZPosition z_position() const { return z_position_; }
};

}
}

#endif
