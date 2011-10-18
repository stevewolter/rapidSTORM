#ifndef DSTORM_TYPES_FLUOROPHORE_H
#define DSTORM_TYPES_FLUOROPHORE_H

#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/dimensionless.hpp>

namespace dStorm {

typedef boost::units::quantity< boost::units::si::dimensionless, int > Fluorophore;

}

#endif
