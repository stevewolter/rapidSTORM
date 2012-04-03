#ifndef DSTORM_GUF_PSF_BASEEVALUATOR_H
#define DSTORM_GUF_PSF_BASEEVALUATOR_H

#include "fwd.h"
#include <Eigen/Core>
#include <nonlinfit/plane/fwd.h>
#include <boost/optional.hpp>
#include <dStorm/polynomial_3d.h>

namespace dStorm {
namespace guf {
namespace PSF {

template <typename Num>
class BaseParameters {
  private:
    const BaseExpression* expr;
  protected:
    Eigen::Array<Num,2,1> sigma, sigmaI, spatial_mean, sigma_deriv;
    Num prefactor, amplitude, transmission;

    void compute_prefactors() { compute_prefactors_(); }

  private:
    virtual boost::optional< Eigen::Array<Num,2,1> > compute_sigma_() = 0;
    virtual void compute_prefactors_() = 0;

  public:
    BaseParameters() : expr(NULL) {}
    BaseParameters( const BaseExpression& expr ) : expr(&expr) {}
    typedef nonlinfit::plane::GenericData< LengthUnit > Data;
    Eigen::Matrix<Num,2,1> compute_sigma() { return *compute_sigma_(); }

    bool prepare_iteration( const Data& data ); 

};

}
}
}

#endif
