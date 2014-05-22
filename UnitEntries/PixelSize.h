#ifndef DSTORM_UNITENTRY_NANORES_H
#define DSTORM_UNITENTRY_NANORES_H

#include "units/nanoresolution.h"
#include "simparm/Entry.h"

namespace dStorm {

typedef simparm::Entry< boost::units::quantity< nanoresolution, float > >
    FloatNanoResolutionEntry;
typedef simparm::Entry< boost::units::quantity< nanometer_pixel_size, float > >
    FloatPixelSizeEntry;

}

#endif
