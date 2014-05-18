#ifndef DSTORM_UNITS_AMPLITUDE_H
#define DSTORM_UNITS_AMPLITUDE_H

#include "boost/units/systems/camera/intensity.hpp"
#include <boost/units/quantity.hpp>

namespace dStorm {

typedef boost::units::quantity<boost::units::camera::intensity,float>
    amplitude;

}

#endif
