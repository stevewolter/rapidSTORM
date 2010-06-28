#ifndef DSTORM_UNITENTRIES_NANOMETRE_H
#define DSTORM_UNITENTRIES_NANOMETRE_H

#include <simparm/UnitEntry.hh>
#include <boost/units/systems/si/length.hpp>
#include <dStorm/units/nanolength.h>

namespace dStorm {
    typedef simparm::UnitEntry< boost::units::si::nanolength, float > 
        FloatNanometreEntry;
}

#endif
