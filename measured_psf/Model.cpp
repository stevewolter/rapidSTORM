#include <boost/test/unit_test.hpp>
#include "Model.h"

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

Model Model::mock() {
	Model rv;
	Image<double,3>::Size size;
	size.fill( 10 * camera::pixel ); //set image dim to 10px
	rv.psf_data = Image<double,3>( size );

        for (int x = 0; x < 10; ++x)
            for (int y = 0; y < 10; ++y)
                for (int z = 0; z < 10; ++z)
                    rv.psf_data(x,y,z) = 2 * x + 3 * y - z;

        rv.x0.fill(5);
	rv.image_x0.fill(5);
	rv.axial_mean = 4.66666;
        rv.pixel_size.fill(1);

	return rv;
}

}
}
