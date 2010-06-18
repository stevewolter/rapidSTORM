#ifndef DSTORM_UNITENTRIES_TEMPERATUREENTRY_H
#define DSTORM_UNITENTRIES_TEMPERATUREENTRY_H

#include <simparm/UnitEntry.hh>
#include <boost/units/systems/temperature/celsius.hpp>

namespace dStorm {

    typedef simparm::UnitEntry< boost::units::celsius::temperature, int > 
        IntCelsiusEntry;
    typedef simparm::UnitEntry< boost::units::celsius::temperature, float > 
        FloatCelsiusEntry;
}

#endif
