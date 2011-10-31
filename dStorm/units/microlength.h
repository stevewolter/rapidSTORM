#ifndef DSTORM_UNITS_MICROLENGTH_H
#define DSTORM_UNITS_MICROLENGTH_H

#include <boost/units/unit.hpp>
#include <boost/units/make_scaled_unit.hpp>
#include <boost/units/systems/si/length.hpp>

namespace boost {
namespace units {
namespace si {

typedef 
    make_scaled_unit<length,
        scale<10, static_rational<-6> > >::type
    microlength;

BOOST_UNITS_STATIC_CONSTANT(micrometer,microlength);
BOOST_UNITS_STATIC_CONSTANT(micrometers,microlength);
BOOST_UNITS_STATIC_CONSTANT(micrometre,microlength);
BOOST_UNITS_STATIC_CONSTANT(micrometres,microlength);

}

}
}

#endif
