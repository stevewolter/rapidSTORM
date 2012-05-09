#ifndef DSTORM_PSF_NO3D_H
#define DSTORM_PSF_NO3D_H

#include "BaseExpression.h"
#include <boost/optional/optional.hpp>
#include <nonlinfit/append.h>

namespace dStorm {
namespace gaussian_psf {

class No3D
: public BaseExpression,
  public nonlinfit::access_parameters<No3D>
{
    template <class Num, typename Expression, int Size> friend class JointEvaluator;
    template <class Num, typename Expression> friend class Parameters;

    template <typename Type> friend class nonlinfit::access_parameters;
    template <int Index> double& access( BestSigma<Index> ) { return best_sigma[Index]; }
    using BaseExpression::access;

    Eigen::Array<double,2,1> best_sigma;

    dStorm::Image<double,3>

  public:
    typedef boost::mpl::vector<
        nonlinfit::Xs<0,LengthUnit>, nonlinfit::Xs<1,LengthUnit>,
        Mean<0>, Mean<1>, MeanZ,
        Amplitude, Prefactor > Variables;

    No3D& copy( const SingleKernelModel& f ) { return *this = dynamic_cast<const No3D&>(f); }

    Eigen::Matrix< quantity<LengthUnit>, 2, 1 > get_sigma() const;

    typedef No3D PSF;
    using nonlinfit::access_parameters<No3D>::operator();
    using nonlinfit::access_parameters<No3D>::get;
};

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


}
}

#endif
