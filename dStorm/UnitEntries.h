#ifndef DSTORM_UNIT_ENTRIES_H
#define DSTORM_UNIT_ENTRIES_H

#include <simparm/UnitEntry.hh>
#include <dStorm/units/nanolength.h>
#include <cs_units/camera/length.hpp>

#include <dStorm/UnitEntries/PixelEntry.h>
#include <dStorm/UnitEntries/ADC.h>

namespace dStorm {
    typedef simparm::UnitEntry< boost::units::si::nanolength, double > 
        NanometreEntry;
    typedef simparm::UnitEntry< cs_units::camera::intensity, float > 
        ADCEntry;
}

#endif
