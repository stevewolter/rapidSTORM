#ifndef DSTORM_UNITENTRIES_HERTZ_H
#define DSTORM_UNITENTRIES_HERTZ_H

#include <boost/units/make_scaled_unit.hpp>
#include <simparm/UnitEntry.hh>
#include <dStorm/units/megafrequency.h>

namespace dStorm {

    typedef simparm::UnitEntry< boost::units::si::megafrequency, float>
        FloatMegahertzEntry;
    typedef simparm::UnitEntry< boost::units::si::frequency, float > 
        FloatHertzEntry;

}

#endif
