#ifndef DSTORM_UNITENTRIES_FRAMEENTRY_H
#define DSTORM_UNITENTRIES_FRAMEENTRY_H

#include "namespaces.h"
#include <simparm/Entry.h>
#include <boost/units/systems/camera/time.hpp>

namespace dStorm {

    typedef simparm::Entry< boost::units::quantity< camera::time, float > > 
        FloatFrameEntry;
    typedef simparm::Entry< boost::units::quantity< camera::time, int > > 
        IntFrameEntry;
}

#endif
