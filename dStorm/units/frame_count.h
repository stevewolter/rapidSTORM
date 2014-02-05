#ifndef DSTORM_UNITS_FRAME_COUNT_H
#define DSTORM_UNITS_FRAME_COUNT_H

#include "dStorm/namespaces.h"
#include <boost/units/systems/camera/time.hpp>
#include <boost/units/quantity.hpp>

namespace dStorm {

typedef quantity<camera::time,int> frame_count;
typedef frame_count frame_index;

}

#endif
