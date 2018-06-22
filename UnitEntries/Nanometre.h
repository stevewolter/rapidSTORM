#ifndef DSTORM_UNITENTRIES_NANOMETRE_H
#define DSTORM_UNITENTRIES_NANOMETRE_H

#include <simparm/BoostUnits.h>
#include <simparm/Entry.h>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/quantity.hpp>
#include "units/nanolength.h"

namespace dStorm {
    typedef simparm::Entry< boost::units::quantity< boost::units::si::nanolength, float > > 
        FloatNanometreEntry;
}

#endif
