#ifndef DSTORM_GUF_PSF_BASEEVALUATOR_H
#define DSTORM_GUF_PSF_BASEEVALUATOR_H

#include "fwd.h"
#include <Eigen/Core>
#include <nonlinfit/plane/fwd.h>

namespace dStorm {
namespace guf {
namespace PSF {

template <typename Num, typename Expression>
class BaseEvaluator {
  private:
    const Expression* expr;
  protected:
    Eigen::Array<Num,2,1> sigma, sigmaI, spatial_mean;
    Eigen::Matrix<Num,2,1>
        z_deriv_prefactor, delta_z_deriv_prefactor, relative_z;
    Num prefactor, amplitude, wavelength, transmission;

  public:
    BaseEvaluator() {}
    BaseEvaluator( const Expression& expr ) : expr(&expr) {}
    typedef nonlinfit::plane::GenericData< LengthUnit > Data;
    Eigen::Matrix<Num,2,1> get_sigma() const;
    void compute_prefactors();

    bool prepare_iteration( const Data& data ); 

};

}
}
}

#endif
