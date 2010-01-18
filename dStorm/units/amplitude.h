#ifndef DSTORM_UNITS_AMPLITUDE_H
#define DSTORM_UNITS_AMPLITUDE_H

#include <cs_units/camera/intensity.hpp>
#include <boost/units/quantity.hpp>

namespace dStorm {

typedef boost::units::quantity<cs_units::camera::intensity,float>
    amplitude;

}

#endif
