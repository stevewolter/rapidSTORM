#include <boost/test/unit_test.hpp>
#include "Model.h"

namespace dStorm
{
namespace measured_psf
{

    quantity<si::length> get_fluorophore_position(int Dim) const
    {
        switch (Dim)
        {
        case 0:
            return quantity<si::length>( (*this)( Mean<0>() ));
        case 1:
            return quantity<si::length>( (*this)( Mean<1>() ));
        case 2:
            throw std::logic_error("Tried to access Z coordinate on 2D model"); //but needed for LocalizationChecker
        default:
            throw std::logic_error("Unconsidered dimension");
        }
    }

    void set_fluorophore_position(int Dim, quantity<si::length> length)
    {
        switch (Dim)
        {
        case 0:
            (*this)( gaussian_psf::Mean<0>() ) = length ;
        case 1:
            (*this)( gaussian_psf::Mean<1>() ) = length ;
        default:
            throw std::logic_error("Unconsidered dimension");
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

    Eigen::Matrix< quantity<gaussian_psf::LengthUnit>, 2, 1 > get_sigma() const
    {
        Eigen::Matrix< quantity<gaussian_psf::LengthUnit>, 2, 1 > m;
        return m.Zero();
    }
}
}
