#ifndef DSTORM_UNITENTRIES_TIMEENTRY_H
#define DSTORM_UNITENTRIES_TIMEENTRY_H

#include <simparm/UnitEntry.hh>
#include <boost/units/systems/si/time.hpp>
#include <dStorm/units/microtime.h>

namespace dStorm {

    typedef simparm::UnitEntry< boost::units::si::microtime, float > 
        FloatMicrosecondsEntry;
    typedef simparm::UnitEntry< boost::units::si::time, float > 
        FloatTimeEntry;
}

#endif
