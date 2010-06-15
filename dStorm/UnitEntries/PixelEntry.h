#ifndef DSTORM_UNITENTRIES_PIXELENTRY_H
#define DSTORM_UNITENTRIES_PIXELENTRY_H

#include <simparm/UnitEntry.hh>
#include <dStorm/units/nanolength.h>
#include <cs_units/camera/length.hpp>

namespace dStorm {

    typedef simparm::UnitEntry< cs_units::camera::length, int > 
        IntPixelEntry;
    typedef simparm::UnitEntry< cs_units::camera::length, float > 
        FloatPixelEntry;

}

#endif
