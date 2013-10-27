#ifndef CSUNITS_CAMERA_FRAME_RATE_HPP
#define CSUNITS_CAMERA_FRAME_RATE_HPP

#include "base.hpp"
#include "time.hpp"
#include <boost/units/systems/si/time.hpp>
#include <boost/units/static_constant.hpp>

namespace boost {
namespace units {
namespace camera {

typedef divide_typeof_helper< time, si::time >::type frame_rate;

BOOST_UNITS_STATIC_CONSTANT(frames_per_second,frame_rate);
BOOST_UNITS_STATIC_CONSTANT(fps,frame_rate);

}
}
}

#endif
