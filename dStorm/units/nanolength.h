#ifndef DSTORM_UNITS_NANOLENGTH_H
#define DSTORM_UNITS_NANOLENGTH_H

#include <boost/units/unit.hpp>
#include <boost/units/make_scaled_unit.hpp>
#include <boost/units/systems/si/length.hpp>

namespace boost {
namespace units {
namespace si {

typedef 
    make_scaled_unit<length,
        scale<10, static_rational<-9> > >::type
    nanolength;

BOOST_UNITS_STATIC_CONSTANT(nanometer,nanolength);
BOOST_UNITS_STATIC_CONSTANT(nanometers,nanolength);
BOOST_UNITS_STATIC_CONSTANT(nanometre,nanolength);
BOOST_UNITS_STATIC_CONSTANT(nanometres,nanolength);

}
}
}


#endif
