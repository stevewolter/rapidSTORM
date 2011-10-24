#include "debug.h"
#include "JobInfo.h"
#include <dStorm/ImageTraits.h>

namespace dStorm {
namespace engine {

int JobInfo::mask_size( int dimension, int plane ) const {
    int mask_size = round( mask_size_factor * sigma(dimension, plane) / camera::pixel );
    DEBUG("Mask size for dimension " << dimension << " and plane " << plane << " is " << mask_size);
    return mask_size;
}

boost::units::quantity< boost::units::si::length > JobInfo::mask_size_in_si( int dimension, int plane ) const
{
    assert( traits.psf_size()[dimension] > 0 * boost::units::si::meter );
    assert( traits.fluorophores.at(0).wavelength > 0 * boost::units::si::meter );
    quantity<si::length> max_wavelength = traits.fluorophores.at(0).wavelength;
    for ( size_t i = 1; i < traits.fluorophores.size(); ++i )
        max_wavelength = std::max( max_wavelength, traits.fluorophores.at(i).wavelength );
    boost::units::quantity< boost::units::si::length > max_sigma =
        traits.psf_size()[dimension] * max_wavelength / traits.fluorophores.at(0).wavelength;
    DEBUG( "Max Sigma for dimension " << dimension << " and plane " << plane << " is " << max_sigma );
    return max_sigma * mask_size_factor;
}

boost::units::quantity< boost::units::si::length > JobInfo::sigma_in_si( int dimension, int plane ) const
{
    assert( traits.psf_size()[dimension] > 0 * boost::units::si::meter );
    assert( traits.fluorophores.at(fluorophore).wavelength > 0 * boost::units::si::meter );
    assert( traits.fluorophores.at(0).wavelength > 0 * boost::units::si::meter );
    float factor = (traits.fluorophores.at(fluorophore).wavelength / traits.fluorophores.at(0).wavelength);
    boost::units::quantity< boost::units::si::length > sigma =
        traits.psf_size()[dimension] * factor;
    DEBUG( "Sigma for dimension " << dimension << " and plane " << plane << " is " << sigma );
    return sigma;
}

boost::units::quantity< boost::units::camera::length > JobInfo::sigma( int dimension, int plane ) const
{
    DEBUG( "Sigma for dimension " << dimension << " and plane " << plane << " is " << sigma );
    return traits.plane(plane).length_in_image_space(dimension, quantity<si::length,float>(sigma_in_si(dimension,plane)));
}

}
}