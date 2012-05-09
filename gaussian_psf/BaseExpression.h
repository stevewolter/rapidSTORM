#ifndef DSTORM_FITTER_PSF_MODEL_H
#define DSTORM_FITTER_PSF_MODEL_H

#include "fwd.h"
#include "parameters.h"
#include "SingleKernelModel.h"

#include <Eigen/Core>
#include <boost/mpl/vector.hpp>
#include <boost/units/quantity.hpp>
#include <nonlinfit/Lambda.h>
#include <boost/units/Eigen/Core>
#include <nonlinfit/access_parameters.hpp>


namespace dStorm {
namespace gaussian_psf {

using namespace nonlinfit;
using namespace boost::units;

struct BaseExpression
: public nonlinfit::access_parameters< BaseExpression >,
  public SingleKernelModel
{
    typedef Micrometers LengthUnit;

    typedef boost::units::quantity< boost::units::multiply_typeof_helper<
        Micrometers, Micrometers >::type > PixelSize;

    BaseExpression();
    virtual ~BaseExpression();
//    virtual Eigen::Matrix< quantity<LengthUnit>, 2, 1 > get_sigma() const = 0;
//    virtual BaseExpression& copy( const BaseExpression& ) = 0;

    using nonlinfit::access_parameters< BaseExpression >::operator();
    using nonlinfit::access_parameters< BaseExpression >::get;


    void allow_leaving_ROI( bool do_allow ) { may_leave_roi = do_allow; }

     quantity<si::length> get_fluorophore_position(int Dim) const
        {
            if (Dim==0) return quantity<si::length>( (*this)( Mean<0>() ));
            return quantity<si::length>( (*this)( Mean<1>() ));
        }


     double intensity() const
        {
             return (((quantity<si::dimensionless>)(*this)(gaussian_psf::Amplitude() )) *  ((quantity<si::dimensionless>)(*this)(gaussian_psf::Prefactor() ) ));
        }

     quantity<si::dimensionless> get_amplitude() const
        {
            return (quantity<si::dimensionless>)(*this)(gaussian_psf::Amplitude() );
        }

	void set_amplitude(quantity<si::dimensionless> amp)
	{
	  (*this)(gaussian_psf::Amplitude())= amp;
	}

//     gaussian_psf::MeanZ get_MeanZ()
//    {
//        return  (gaussian_psf::MeanZ)(*this)( gaussian_psf::MeanZ() );
//    }


    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  protected:
    Eigen::Array<double,2,1> spatial_position, spatial_mean;
    double amplitude, transmission;
    bool may_leave_roi;
    typedef boost::units::multiply_typeof_helper< LengthUnit, LengthUnit >::type
        PixelSizeUnit;
    typedef boost::mpl::vector<
        nonlinfit::Xs<0,LengthUnit>, nonlinfit::Xs<1,LengthUnit>,
        Mean<0>, Mean<1>,
        Amplitude, Prefactor
    > Variables;

    template <int Index> quantity<si::length> get_fluorophore_position_x() const
        {
             return quantity<si::length>( (*this)( Mean<0>() ));
        }
    template <class Num> friend class BaseParameters;
    template <class Num, class Expression> friend class Parameters;
    template <class Num, class Expression, int Size> friend class JointEvaluator;
    template <class Num, class Expression, int Size> friend class DisjointEvaluator;

    template <typename Type> friend class nonlinfit::access_parameters;
    template <int Index> double& access( nonlinfit::Xs<Index,LengthUnit> ) {
        return spatial_position[Index];
    }
    template <int Index> double& access( Mean<Index> ) { return spatial_mean[Index]; }
    double& access( Amplitude ) { return amplitude; }
    double& access( Prefactor ) { return transmission; }

    bool form_parameters_are_sane() const;

    typedef Eigen::Matrix< quantity<Micrometers>, 2, 1, Eigen::DontAlign > Bound;
    bool mean_within_range( const Bound& lower, const Bound& upper ) const;
    bool sigma_is_negligible( quantity<PixelSizeUnit> pixel_size ) const;

};

}
}

#endif
