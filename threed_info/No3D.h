#ifndef DSTORM_TRAITS_NO3D_H
#define DSTORM_TRAITS_NO3D_H

#include <boost/make_shared.hpp>

#include "threed_info/DepthInfo.h"

namespace dStorm {
namespace threed_info {

class No3D : public DepthInfo {
  public:
    No3D(double fwhm) : sigma(fwhm / 2.3548f) {}
    double fwhm() const { return sigma * 2.3548f; }
    double stddev() const { return sigma; }

  private:
    Sigma sigma;
    std::string config_name_() const OVERRIDE { return "No3D"; }
    Sigma get_sigma_( ZPosition z ) const OVERRIDE { return sigma; }
    SigmaDerivative get_sigma_deriv_( ZPosition ) const OVERRIDE;
    ZRange z_range_() const OVERRIDE { return ZRange(); }
    std::ostream& print_( std::ostream& o ) const OVERRIDE; 
    bool provides_3d_info_() const OVERRIDE { return false; }
};

}
}

#endif
