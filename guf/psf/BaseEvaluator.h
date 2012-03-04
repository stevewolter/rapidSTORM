#ifndef DSTORM_GUF_PSF_BASEEVALUATOR_H
#define DSTORM_GUF_PSF_BASEEVALUATOR_H

#include "fwd.h"
#include <Eigen/Core>
#include <nonlinfit/plane/fwd.h>
#include <boost/optional.hpp>

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

template <typename Num, typename Expression> class Parameters;

template <typename Num>
class Parameters< Num, No3D >
: public BaseParameters<Num>
{
    const No3D* expr;

    boost::optional< Eigen::Array<Num,2,1> > compute_sigma_();
    void compute_prefactors_();

  public:
    Parameters() {}
    Parameters( const No3D& expr ) : BaseParameters<Num>(expr), expr(&expr) {}
};

template <typename Num>
class Parameters< Num, Polynomial3D >
: public BaseParameters<Num>
{
    boost::optional< Eigen::Array<Num,2,1> > compute_sigma_();
    void compute_prefactors_();

  protected:
    const Polynomial3D* expr;
    Eigen::Array<Num,2,1> z_deriv_prefactor, relative_z, threed_factor;
    Eigen::Array<Num,2,5> delta_z_deriv_prefactor;
  public:
    Parameters() {}
    Parameters( const Polynomial3D& expr ) : BaseParameters<Num>(expr), expr(&expr) {}
};

}
}
}

#endif
