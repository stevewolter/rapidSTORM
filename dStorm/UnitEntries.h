#ifndef DSTORM_UNIT_ENTRIES_H
#define DSTORM_UNIT_ENTRIES_H

#include <simparm/UnitEntry.hh>
#include <dStorm/units/nanolength.h>
#include <cs_units/camera/length.hpp>
#include <cs_units/camera/intensity.hpp>

namespace dStorm {
    typedef simparm::UnitEntry< boost::units::si::nanolength, double > 
        NanometreEntry;
    typedef simparm::UnitEntry< cs_units::camera::length, int > 
        IntPixelEntry;
    typedef simparm::UnitEntry< cs_units::camera::length, float > 
        FloatPixelEntry;
    typedef simparm::UnitEntry< cs_units::camera::intensity, float > 
        ADCEntry;
}

#endif
