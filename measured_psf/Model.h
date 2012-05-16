#ifndef DSTORM_MSD_PSF_Model_H
#define DSTORM_MSD_PSF_Model_H

#include <boost/optional/optional.hpp>
#include <nonlinfit/access_parameters.hpp>
#include <nonlinfit/append.h>
#include "guf/SingleKernelModel.h"
#include "LengthUnit.h"
#include "parameters.h"
#include <dStorm/Image.h>
#include <dStorm/image/constructors.h>

namespace dStorm {
namespace measured_psf {

template <class Num, int Size> class Evaluator;

using boost::units::quantity;
namespace si = boost::units::si;

class Model
:   public nonlinfit::access_parameters<Model>,
    public guf::SingleKernelModel
{
public:
    template <class Num, int Size> friend class Evaluator;
    template <typename Type> friend class nonlinfit::access_parameters;
    using nonlinfit::access_parameters<Model>::operator();
    using nonlinfit::access_parameters<Model>::get;

    typedef boost::mpl::vector<
        /* These parameters are the three dimension of the search image position. */
        nonlinfit::Xs<0,LengthUnit>, nonlinfit::Xs<1,LengthUnit>, nonlinfit::Xs<2,LengthUnit>,
        /* The three dimensions of the fluorophore position in the search image. */
        Mean<0>, Mean<1>, Mean<2>,
        Amplitude, Prefactor > Variables;

private:
     quantity<si::length> get_fluorophore_position(int Dim) const;
     void set_fluorophore_position(int, quantity<si::length>);
     double intensity() const;
     quantity<si::dimensionless> get_amplitude() const;
     bool has_z_position() const { return true; }
     void set_amplitude(quantity<si::dimensionless> amp);
     Eigen::Matrix< quantity<gaussian_psf::LengthUnit>, 2, 1 > get_sigma() const;
  public:
     static Model mock();

  private:
    Model& copy( const SingleKernelModel& f ) { return *this = dynamic_cast<const Model&>(f); }

    Eigen::Vector3d x0, image_x0;
    /** pixel_size of psf_data in um/px */
    Eigen::Vector3d pixel_size;
    double axial_mean, amplitude, prefactor;
    Image<double,3> psf_data;

    double& access( nonlinfit::Xs<2,LengthUnit> ) { return axial_mean; }
    template <int Index> double& access( Mean<Index> ) { return x0[Index]; }
    double& access( Amplitude ) { return amplitude; }
    double& access( Prefactor ) { return prefactor; }
};

}
}

#endif
