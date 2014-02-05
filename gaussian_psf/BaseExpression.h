#ifndef DSTORM_FITTER_PSF_MODEL_H
#define DSTORM_FITTER_PSF_MODEL_H

#include "gaussian_psf/fwd.h"
#include "gaussian_psf/parameters.h"

#include <Eigen/Core>
#include <boost/mpl/vector.hpp>
#include <nonlinfit/Lambda.h>
#include <nonlinfit/access_parameters.hpp>

namespace dStorm {
namespace gaussian_psf {

using namespace nonlinfit;

struct BaseExpression
: public nonlinfit::access_parameters< BaseExpression >
{
    BaseExpression();
    virtual ~BaseExpression();
    // Returns the PSF standard deviation in micrometers.
    virtual Eigen::Vector2d get_sigma() const = 0;
    virtual BaseExpression& copy( const BaseExpression& ) = 0;

    using nonlinfit::access_parameters< BaseExpression >::operator();
    using nonlinfit::access_parameters< BaseExpression >::get;

    void allow_leaving_ROI( bool do_allow ) { may_leave_roi = do_allow; }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  protected:
    Eigen::Array<double,2,1> spatial_position, spatial_mean;
    double amplitude, transmission;
    bool may_leave_roi;
    typedef boost::mpl::vector< 
        nonlinfit::Xs<0>, nonlinfit::Xs<1>,
        Mean<0>, Mean<1>, 
        Amplitude, Prefactor
    > Variables;

    template <class Num> friend class BaseParameters;
    template <class Num, class Expression> friend class Parameters;
    template <class Num, class Expression, int Size> friend class JointEvaluator;
    template <class Num, class Expression, int Size> friend class DisjointEvaluator;

    template <typename Type> friend class nonlinfit::access_parameters;
    template <int Index> double& access( nonlinfit::Xs<Index> ) {
        return spatial_position[Index];
    }
    template <int Index> double& access( Mean<Index> ) { return spatial_mean[Index]; }
    double& access( Amplitude ) { return amplitude; }
    double& access( Prefactor ) { return transmission; }

    bool form_parameters_are_sane() const;

    typedef Eigen::Matrix< double, 2, 1, Eigen::DontAlign > Bound;
    bool mean_within_range( const Bound& lower, const Bound& upper ) const;
    bool sigma_is_negligible( double pixel_size ) const;

};

}
}

#endif
