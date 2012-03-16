#ifndef DSTORM_UNITS_PERMICROLENGTH_H
#define DSTORM_UNITS_PERMICROLENGTH_H

#include <boost/units/power10.hpp>
#include <boost/units/systems/si/length.hpp>

namespace boost {
namespace units {
namespace si {

typedef 
    power_typeof_helper<
        power10< length, -6 >::type,
        static_rational<-1> >::type
    permicrolength;

BOOST_UNITS_STATIC_CONSTANT(per_micrometer,permicrolength);
BOOST_UNITS_STATIC_CONSTANT(per_micrometers,permicrolength);
BOOST_UNITS_STATIC_CONSTANT(per_micrometre,permicrolength);
BOOST_UNITS_STATIC_CONSTANT(per_micrometres,permicrolength);

}

std::string name_string(const si::permicrolength&);
std::string symbol_string(const si::permicrolength&);

}
}

#endif
