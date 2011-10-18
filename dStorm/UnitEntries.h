#ifndef DSTORM_UNIT_ENTRIES_H
#define DSTORM_UNIT_ENTRIES_H

#include "namespaces.h"
#include <simparm/Entry.hh>
#include "units/nanolength.h"
#include <boost/units/systems/camera/length.hpp>

#include "UnitEntries/PixelEntry.h"
#include "UnitEntries/ADC.h"

namespace dStorm {
    typedef simparm::Entry< quantity< si::nanolength, double > > 
        NanometreEntry;
    typedef simparm::Entry< quantity< camera::intensity, float > > 
        ADCEntry;
}

#endif
