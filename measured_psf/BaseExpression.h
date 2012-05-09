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
namespace measured_psf {

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

    using nonlinfit::access_parameters< BaseExpression >::operator();
    using nonlinfit::access_parameters< BaseExpression >::get;


    void allow_leaving_ROI( bool do_allow ) { may_leave_roi = do_allow; }

     quantity<si::length> get_fluorophore_position(int Dim) const
        {
            switch (Dim) {
                case 0: return quantity<si::length>( (*this)( Mean<0>() ));
                case 1: return quantity<si::length>( (*this)( Mean<1>() ));
                case 2: throw std::logic_error("Tried to access Z coordinate on 2D model");
                default:throw std::logic_error("Unconsidered dimension");
            }
        }


     double intensity() const
        {
             return (((*this)(gaussian_psf::Amplitude() )) *  ((*this)(gaussian_psf::Prefactor() ) ));
        }

     quantity<si::dimensionless> get_amplitude() const
        {
            return (quantity<si::dimensionless>)(*this)(gaussian_psf::Amplitude() );
        }

	void set_amplitude(quantity<si::dimensionless> amp)
	{
	  (*this)(gaussian_psf::Amplitude())= amp;
	}

    bool has_z_position() const { return false; }

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
