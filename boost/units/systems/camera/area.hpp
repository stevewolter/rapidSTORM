#ifndef CSUNITS_CAMERA_AREA_HPP
#define CSUNITS_CAMERA_AREA_HPP

#include "base.hpp"
#include <boost/units/physical_dimensions/area.hpp>
#include <boost/units/static_constant.hpp>

namespace boost {
namespace units {
namespace camera {

typedef boost::units::multiply_typeof_helper<length, length>::type area;

}
}
}

#endif
