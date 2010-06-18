#ifndef DSTORM_UNITS_MICROTIME_H
#define DSTORM_UNITS_MICROTIME_H

#include <boost/units/unit.hpp>
#include <boost/units/make_scaled_unit.hpp>
#include <boost/units/systems/si/time.hpp>

namespace boost {
namespace units {
namespace si {

typedef 
    make_scaled_unit<time,
        scale<10, static_rational<-6> > >::type
    microtime;

BOOST_UNITS_STATIC_CONSTANT(microsecond,microtime);
BOOST_UNITS_STATIC_CONSTANT(microseconds,microtime);

}
}
}

#endif
