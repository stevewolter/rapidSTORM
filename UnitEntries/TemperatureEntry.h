#ifndef DSTORM_UNITENTRIES_TEMPERATUREENTRY_H
#define DSTORM_UNITENTRIES_TEMPERATUREENTRY_H

#include "simparm/Entry.h"
#include <boost/units/systems/temperature/celsius.hpp>

namespace dStorm {

    typedef simparm::Entry< boost::units::quantity< boost::units::celsius::temperature, int > > 
        IntCelsiusEntry;
    typedef simparm::Entry< boost::units::quantity< boost::units::celsius::temperature, float > > 
        FloatCelsiusEntry;
}

#endif
