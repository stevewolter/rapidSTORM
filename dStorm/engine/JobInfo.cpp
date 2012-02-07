#include "debug.h"
#include "JobInfo.h"
#include <dStorm/ImageTraits.h>

namespace dStorm {
namespace engine {

boost::units::quantity< boost::units::si::length > JobInfo::mask_size_in_si( int dimension, int plane ) const
{
    assert( (*traits.plane(plane).psf_size(0))[dimension] > 0 * boost::units::si::meter );
    assert( traits.fluorophores.at(0).wavelength > 0 * boost::units::si::meter );
    quantity<si::length> max_wavelength = traits.fluorophores.at(0).wavelength;
    for ( size_t i = 1; i < traits.fluorophores.size(); ++i )
        max_wavelength = std::max( max_wavelength, traits.fluorophores.at(i).wavelength );
    boost::units::quantity< boost::units::si::length > max_sigma =
        (*traits.plane(plane).psf_size(0))[dimension] * max_wavelength / traits.fluorophores.at(0).wavelength;
    DEBUG( "Max Sigma for dimension " << dimension << " and plane " << plane << " is " << max_sigma );
    return max_sigma * mask_size_factor;
}

}
}
