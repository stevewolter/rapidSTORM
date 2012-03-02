#ifndef DSTORM_FITTER_PSF_MODEL_H
#define DSTORM_FITTER_PSF_MODEL_H

#include "fwd.h"
#include "parameters.h"

#include <Eigen/Core>
#include <boost/mpl/vector.hpp>
#include <boost/units/quantity.hpp>
#include <nonlinfit/Lambda.h>
#include <boost/units/Eigen/Core>
#include <nonlinfit/access_parameters.hpp>

namespace dStorm {
namespace guf {
namespace PSF {

using namespace nonlinfit;
using namespace boost::units;

struct BaseExpression
: public nonlinfit::access_parameters< BaseExpression >
{
    typedef Micrometers LengthUnit;

    typedef boost::units::quantity< boost::units::multiply_typeof_helper<
        Micrometers, Micrometers >::type > PixelSize;

    BaseExpression();
    virtual ~BaseExpression();
    virtual Eigen::Matrix< quantity<LengthUnit>, 2, 1 > get_sigma() const = 0;
    virtual BaseExpression& copy( const BaseExpression& ) = 0;

    using nonlinfit::access_parameters< BaseExpression >::operator();
    using nonlinfit::access_parameters< BaseExpression >::get;

    void allow_leaving_ROI( bool do_allow ) { may_leave_roi = do_allow; }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  protected:
    Eigen::Array<double,2,1> spatial_position, spatial_mean, best_sigma;
    double amplitude, transmission;
    bool may_leave_roi;
    typedef boost::units::multiply_typeof_helper< LengthUnit, LengthUnit >::type
        PixelSizeUnit;
    typedef boost::mpl::vector< 
        nonlinfit::Xs<0,LengthUnit>, nonlinfit::Xs<1,LengthUnit>,
        Mean<0>, Mean<1>, 
        BestSigma<0>, BestSigma<1>, 
        Amplitude, Prefactor
    > Variables;

    template <class Num> friend class BaseParameters;
    template <class Num, class Expression> friend class Parameters;
    template <class Num, class Expression, int Size> friend class JointEvaluator;
    template <class Num, class Expression, int Size> friend class DisjointEvaluator;

    template <typename Type> friend class nonlinfit::access_parameters;
    template <int Index> double& access( nonlinfit::Xs<Index,LengthUnit> ) {
        return spatial_position[Index];
    }
    template <int Index> double& access( Mean<Index> ) { return spatial_mean[Index]; }
    template <int Index> double& access( BestSigma<Index> ) { return best_sigma[Index]; }
    double& access( Amplitude ) { return amplitude; }
    double& access( Prefactor ) { return transmission; }

    bool form_parameters_are_sane() const;

    typedef Eigen::Matrix< quantity<Micrometers>, 2, 1, Eigen::DontAlign > Bound;
    bool mean_within_range( const Bound& lower, const Bound& upper ) const;
    bool sigma_is_negligible( quantity<PixelSizeUnit> pixel_size ) const;

};

}
}
}

#endif
