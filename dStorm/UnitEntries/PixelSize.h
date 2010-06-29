#ifndef DSTORM_UNITENTRY_NANORES_H
#define DSTORM_UNITENTRY_NANORES_H

#include <dStorm/units/nanoresolution.h>
#include <simparm/UnitEntry.hh>

namespace dStorm {

typedef simparm::UnitEntry< nanoresolution, float >
    FloatNanoResolutionEntry;
typedef simparm::UnitEntry< nanometer_pixel_size, float >
    FloatPixelSizeEntry;

}

#endif
