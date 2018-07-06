#ifndef DSTORM_KALMAN_FILTER_UNITS_H
#define DSTORM_KALMAN_FILTER_UNITS_H

#include <boost/units/systems/si/length.hpp>
#include "boost/units/systems/camera/time.hpp"

namespace dStorm {
namespace kalman_filter {

typedef boost::units::si::length observation_unit;
typedef boost::units::multiply_typeof_helper<observation_unit,observation_unit>::type obs_variance_unit;
typedef boost::units::camera::time time_unit;
typedef boost::units::divide_typeof_helper< observation_unit, time_unit >::type speed_unit;
typedef boost::units::divide_typeof_helper< obs_variance_unit, time_unit >::type diffusion_unit;
typedef boost::units::divide_typeof_helper< 
    boost::units::multiply_typeof_helper<speed_unit,speed_unit>::type,
    time_unit >::type mobility_unit;

}
}

#endif
