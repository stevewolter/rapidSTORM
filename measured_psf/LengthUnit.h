#ifndef MSD_PSF_LENGTH_UNIT_H
#define MSD_PSF_LENGTH_UNIT_H

#include <boost/units/power10.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/systems/camera/intensity.hpp>

namespace dStorm {
namespace measured_psf {

typedef boost::units::power10< boost::units::si::length, -6 >::type
    Micrometers;
typedef Micrometers LengthUnit;
typedef boost::units::multiply_typeof_helper< Micrometers, Micrometers >::type AreaUnit;
typedef boost::units::camera::intensity
    ADCounts;

}
}

#endif
