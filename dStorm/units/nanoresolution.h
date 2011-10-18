#ifndef DSTORM_UNITS_NANORES_H
#define DSTORM_UNITS_NANORES_H

#include "../namespaces.h"
#include "nanolength.h"
#include <boost/units/systems/camera/length.hpp>

namespace dStorm {
typedef boost::units::divide_typeof_helper<
                    camera::length, si::nanolength >::type
    nanoresolution;
typedef boost::units::divide_typeof_helper<
                    si::nanolength, camera::length >::type
    nanometer_pixel_size;

BOOST_UNITS_STATIC_CONSTANT(pixels_per_nanometer,nanoresolution);
}

namespace boost {
namespace units {
std::string name_string(const dStorm::nanoresolution&);
std::string name_string(const dStorm::nanometer_pixel_size&);
std::string symbol_string(const dStorm::nanoresolution&);
std::string symbol_string(const dStorm::nanometer_pixel_size&);
}
}

#endif
