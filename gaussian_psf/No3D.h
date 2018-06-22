#ifndef DSTORM_PSF_NO3D_H
#define DSTORM_PSF_NO3D_H

#include "gaussian_psf/BaseExpression.h"
#include <nonlinfit/append.h>

namespace dStorm {
namespace gaussian_psf {

class No3D
: public BaseExpression,
  public nonlinfit::access_parameters<No3D>
{
    template <class Num, typename Expression> friend class BaseEvaluator;
    template <class Num, typename Expression, int Size> friend class JointEvaluator;
    template <class Num, typename Expression, int Size> friend class DisjointEvaluator;
    template <class Num, typename Expression> friend class Parameters;

    template <typename Type> friend class nonlinfit::access_parameters;
    template <int Index> double& access( BestSigma<Index> ) { return best_sigma[Index]; }
    using BaseExpression::access;

    Eigen::Array<double,2,1> best_sigma;
  
  public:
    typedef nonlinfit::append<
            BaseExpression::Variables,
            boost::mpl::vector< BestSigma<0>, BestSigma<1> > >::type
        Variables;

    No3D& copy( const BaseExpression& f ) { return *this = dynamic_cast<const No3D&>(f); }

    // Returns the PSF standard deviation in micrometers.
    Eigen::Vector2d get_sigma() const;

    typedef No3D PSF;
    using nonlinfit::access_parameters<No3D>::operator();
    using nonlinfit::access_parameters<No3D>::get;
};

template <typename Num>
class Parameters< Num, No3D >
: public BaseParameters<Num>
{
    const No3D* expr;

    Eigen::Array<Num,2,1> compute_sigma_();
    void compute_prefactors_();

  public:
    Parameters() {}
    Parameters( const No3D& expr ) : BaseParameters<Num>(expr), expr(&expr) {}
};


}
}

#endif
