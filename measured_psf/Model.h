#ifndef DSTORM_MSD_PSF_Model_H
#define DSTORM_MSD_PSF_Model_H

#include <boost/optional/optional.hpp>
#include <nonlinfit/append.h>

namespace dStorm {
namespace measured_psf {

class Model
:   public nonlinfit::access_parameters<Model>,
    public guf::SingleKernelModel
{
    template <class Num, typename Expression, int Size> friend class JointEvaluator;
    template <typename Type> friend class nonlinfit::access_parameters;
    using nonlinfit::access_parameters<Model>::operator();
    using nonlinfit::access_parameters<Model>::get;

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


  private:
    typedef boost::mpl::vector<
        nonlinfit::Xs<0,LengthUnit>, nonlinfit::Xs<1,LengthUnit>,
        Mean<0>, Mean<1>, MeanZ, Z_Position,
        Amplitude, Prefactor > Variables;

    Model& copy( const SingleKernelModel& f ) { return *this = dynamic_cast<const Model&>(f); }

    Eigen::Vector3d x0, image_x0;
    Eigen::Vector3d pixel_size;
    double z_pos;

    double & access( Mean<0> ) { return x0[0]; }
};

}
}

#endif
