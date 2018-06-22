#ifndef DSTORM_UNITENTRIES_HERTZ_H
#define DSTORM_UNITENTRIES_HERTZ_H

#include <boost/units/make_scaled_unit.hpp>
#include <simparm/Entry.h>
#include "units/megafrequency.h"

namespace dStorm {

    typedef simparm::Entry< boost::units::quantity< boost::units::si::megafrequency, float > >
        FloatMegahertzEntry;
    typedef simparm::Entry< boost::units::quantity< boost::units::si::frequency, float > > 
        FloatHertzEntry;

}

#endif
