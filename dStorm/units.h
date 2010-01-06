#ifndef DSTORM_UNITS_H
#define DSTORM_UNITS_H

#include "camera_units.h"
#include <boost/units/systems/si/time.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/systems/si/prefixes.hpp>
#include <boost/units/make_scaled_unit.hpp>
#include <boost/units/quantity.hpp>

namespace dStorm {

using boost::units::quantity;

namespace si {
    using namespace boost::units::si;
}

typedef quantity<camera::time, int> frame_count;
typedef frame_count frame_index;
typedef quantity<camera::length, int> pixel_count;

}

#endif
