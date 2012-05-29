#include <boost/test/unit_test.hpp>
#include "Model.h"
#include "guf/GaussImage.h"


namespace dStorm
{
namespace measured_psf
{

quantity<si::length> Model::get_fluorophore_position(int Dim) const
{
	return quantity<si::length>( quantity<Mean<0>::Unit>::from_value( x0[Dim] ) );
}

void Model::set_fluorophore_position(int Dim, quantity<si::length> length)
{
	x0[Dim] = quantity< Mean<0>::Unit >( length ).value();
}

double Model::intensity() const
{
    return (((*this)(Amplitude() )) *  ((*this)(Prefactor() ) ));
}

quantity<si::dimensionless> Model::get_amplitude() const
{
    return (quantity<si::dimensionless>)(*this)(Amplitude() );
}

void Model::set_amplitude(quantity<si::dimensionless> amp)
{
    (*this)(Amplitude())= amp;
}

Eigen::Matrix< quantity<gaussian_psf::LengthUnit>, 2, 1 > Model::get_sigma() const
{
    Eigen::Matrix< quantity<gaussian_psf::LengthUnit>, 2, 1 > m;
    return m.Zero();
}

void Model::set_calibration_data(Eigen::Vector3d& image_x, Image<double,3>& psf_data_image, Eigen::Vector3d pixel_size)
{
    (*this).image_x0 = image_x;
    (*this).psf_data = psf_data_image;
    (*this).pixel_size = pixel_size;
}

void Model::set_fixed_calibration_data()
{
    Eigen::Vector3d image_x0;
    Eigen::Vector3d pixel_size;
    image_x0 << 0.9, 0.9, 1.5;
    pixel_size << 0.06, 0.06, 0.06;
    dStorm::Image<double,3>::Size size;
    size[0]= 30 * camera::pixel;
    size[1]= 30 * camera::pixel;
    size[2]= 50 * camera::pixel;
    Image <double,3> psf_calib_image = Image<double,3>( size );
    double correction_factor = 12.76615297;

    for (int x = 0; x < 30; ++x)
        for (int y = 0; y < 30; ++y)
            for (int z = 0; z < 50; ++z)
                psf_calib_image(x,y,z) = correction_factor * dStorm::guf::psf_calib_image_test[x][y][z];

    (*this).image_x0 = image_x0;
    (*this).psf_data = psf_calib_image;
    (*this).pixel_size = pixel_size;
}

Model Model::mock() {
    Model rv;
    Image<double,3>::Size size;
    size.fill( 10 * camera::pixel );
    rv.psf_data = Image<double,3>( size );

    for (int x = 0; x < 10; ++x)
        for (int y = 0; y < 10; ++y)
            for (int z = 0; z < 10; ++z)
                rv.psf_data(x,y,z) = 2 * x + 3 * y - z;

    rv.x0.fill(6);
    rv.image_x0.fill(5);
    rv.axial_mean = 4;
    rv.pixel_size.fill(0.5);
    rv.amplitude = 2.5;
    rv.prefactor = 2;

    return rv;
}

}
}
