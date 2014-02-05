#ifndef DSTORM_UNIT_ENTRIES_H
#define DSTORM_UNIT_ENTRIES_H

#include "dStorm/namespaces.h"
#include <simparm/Entry.h>
#include "dStorm/units/nanolength.h"
#include <boost/units/systems/camera/length.hpp>

#include "dStorm/UnitEntries/PixelEntry.h"
#include "dStorm/UnitEntries/ADC.h"

namespace dStorm {
    typedef simparm::Entry< quantity< si::nanolength, double > > 
        NanometreEntry;
    typedef simparm::Entry< quantity< camera::intensity, float > > 
        ADCEntry;
}

#endif
