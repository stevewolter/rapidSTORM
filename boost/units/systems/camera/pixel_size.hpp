#ifndef CSUNITS_CAMERA_PIXEL_SIZE2_HPP
#define CSUNITS_CAMERA_PIXEL_SIZE2_HPP

#include "base.hpp"
#include "length.hpp"
#include <boost/units/systems/si/length.hpp>
#include <boost/units/static_constant.hpp>

namespace boost {
namespace units {
namespace camera {

typedef divide_typeof_helper< 
    si::length, length >::type pixel_size;

BOOST_UNITS_STATIC_CONSTANT(meters_per_pixel,pixel_size);

}
}
}

#endif
