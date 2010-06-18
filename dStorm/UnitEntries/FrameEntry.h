#ifndef DSTORM_UNITENTRIES_FRAMEENTRY_H
#define DSTORM_UNITENTRIES_FRAMEENTRY_H

#include <simparm/UnitEntry.hh>
#include <cs_units/camera/time.hpp>

namespace dStorm {

    typedef simparm::UnitEntry< cs_units::camera::time, float > 
        FloatFrameEntry;
    typedef simparm::UnitEntry< cs_units::camera::time, int > 
        IntFrameEntry;
}

#endif
