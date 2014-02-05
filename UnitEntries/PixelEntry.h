#ifndef DSTORM_UNITENTRIES_PIXELENTRY_H
#define DSTORM_UNITENTRIES_PIXELENTRY_H

#include "namespaces.h"
#include <simparm/Entry.h>
#include "units/nanolength.h"
#include <boost/units/systems/camera/length.hpp>
#include <boost/units/systems/camera/area.hpp>

namespace dStorm {

    typedef simparm::Entry< boost::units::quantity< camera::length, int > > 
        IntPixelEntry;
    typedef simparm::Entry< boost::units::quantity< camera::length, float > > 
        FloatPixelEntry;

    typedef simparm::Entry< boost::units::quantity< camera::area, int > > 
        IntPixelAreaEntry;
}

#endif
