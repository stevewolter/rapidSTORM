#ifndef DSTORM_MSD_PSF_Model_H
#define DSTORM_MSD_PSF_Model_H

#include <boost/optional/optional.hpp>
#include <nonlinfit/access_parameters.h>
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

     quantity<si::length> get_fluorophore_position(int Dim) const;
     void set_fluorophore_position(int, quantity<si::length>);
     double intensity() const;
     quantity<si::dimensionless> get_amplitude() const;
     bool has_z_position() const { return true; }
     void set_amplitude(quantity<si::dimensionless> amp);
     Eigen::Matrix< quantity<gaussian_psf::LengthUnit>, 2, 1 > get_sigma() const
     {
         Eigen::Matrix< quantity<gaussian_psf::LengthUnit>, 2, 1 > m;
         return m.Zero();
     }

  private:
    typedef boost::mpl::vector<
        nonlinfit::Xs<0,LengthUnit>, nonlinfit::Xs<1,LengthUnit>, nonlinfit::Xs<2,LengthUnit>,
        Mean<0>, Mean<1>, Mean<2>,  
        Amplitude, Prefactor > Variables;

    Model& copy( const SingleKernelModel& f ) { return *this = dynamic_cast<const Model&>(f); }

    Eigen::Vector3d x0, image_x0;
    Eigen::Vector3d pixel_size;
    double axial_mean;

    template <int Index> double& access( Mean<Index> ) { return x0[Index]; }

   // double & access( Z_Position ) { return z_pos; }
    };

}
}

#endif
