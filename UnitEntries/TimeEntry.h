#ifndef DSTORM_UNITENTRIES_TIMEENTRY_H
#define DSTORM_UNITENTRIES_TIMEENTRY_H

#include "simparm/Entry.h"
#include <boost/units/systems/si/time.hpp>
#include "units/microtime.h"

namespace dStorm {

    typedef simparm::Entry< boost::units::quantity< boost::units::si::microtime, float > > 
        FloatMicrosecondsEntry;
    typedef simparm::Entry< boost::units::quantity< boost::units::si::time, float > > 
        FloatTimeEntry;
}

#endif
