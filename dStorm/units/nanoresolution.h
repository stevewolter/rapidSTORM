#ifndef DSTORM_UNITS_NANORES_H
#define DSTORM_UNITS_NANORES_H

#include <dStorm/units/nanolength.h>
#include <cs_units/camera/length.hpp>

namespace dStorm {
typedef boost::units::divide_typeof_helper<
                    cs_units::camera::length,
                    boost::units::si::nanolength
                >::type
    nanoresolution;
typedef boost::units::divide_typeof_helper<
                    boost::units::si::nanolength,
                    cs_units::camera::length
                >::type
    nanometer_pixel_size;

BOOST_UNITS_STATIC_CONSTANT(pixels_per_nanometer,nanoresolution);
}

#endif
