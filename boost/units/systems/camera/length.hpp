#ifndef CSUNITS_CAMERA_LENGTH_HPP
#define CSUNITS_CAMERA_LENGTH_HPP

#include "base.hpp"
#include <boost/units/static_constant.hpp>

namespace boost {
namespace units {
namespace camera {

typedef pixel_base_unit::unit_type length;

BOOST_UNITS_STATIC_CONSTANT(pixel,length);
BOOST_UNITS_STATIC_CONSTANT(pixels,length);

}
}
}


#endif
