#ifndef DSTORM_UNITS_MEGAFREQUENCY_H
#define DSTORM_UNITS_MEGAFREQUENCY_H

#include <boost/units/unit.hpp>
#include <boost/units/make_scaled_unit.hpp>
#include <boost/units/systems/si/frequency.hpp>

namespace boost {
namespace units {
namespace si {

typedef 
    make_scaled_unit<frequency,
        scale<10, static_rational<6> > >::type
    megafrequency;

BOOST_UNITS_STATIC_CONSTANT(megahertz,megafrequency);

}
}
}

#endif
