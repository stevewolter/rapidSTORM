#ifndef DSTORM_UNITS_FRAME_COUNT_H
#define DSTORM_UNITS_FRAME_COUNT_H

#include <cs_units/camera/time.hpp>
#include <boost/units/quantity.hpp>

namespace dStorm {

typedef boost::units::quantity<cs_units::camera::time,int>
    frame_count;
typedef frame_count frame_index;

}

#endif
