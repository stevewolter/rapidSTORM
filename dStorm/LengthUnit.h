// This file defines the universal length unit of rapidSTORM - micrometer -,
// and offers conversion routines.
#ifndef DSTORM_LENGTH_UNIT_H
#define DSTORM_LENGTH_UNIT_H

#include <boost/units/quantity.hpp>
#include "dStorm/units/microlength.h"

namespace dStorm {

typedef boost::units::si::microlength LengthUnit;

template <typename Unit, typename Quantity>
Quantity ToLengthUnit(const boost::units::quantity<Unit,Quantity>& quantity) {
    return boost::units::quantity<LengthUnit>(quantity).value();
}

}

#endif
