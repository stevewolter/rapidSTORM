#ifndef CSUNITS_CAMERA_PIXEL_SIZE_HPP
#define CSUNITS_CAMERA_PIXEL_SIZE_HPP

#include "boost/units/systems/camera/base.hpp"
#include "boost/units/systems/camera/length.hpp"
#include <boost/units/systems/si/length.hpp>
#include <boost/units/static_constant.hpp>

namespace boost {
namespace units {
namespace camera {

typedef divide_typeof_helper< 
    length, si::length >::type resolution;

BOOST_UNITS_STATIC_CONSTANT(pixels_per_meter,resolution);

}
}
}

#endif
