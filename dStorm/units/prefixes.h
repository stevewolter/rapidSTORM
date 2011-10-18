#ifndef DSTORM_UNITS_SI_PREFIXES_HPP
#define DSTORM_UNITS_SI_PREFIXES_HPP

#include <boost/units/make_scaled_unit.hpp>

namespace boost {
namespace units {
namespace si {

#ifdef BOOST_UNITS_METRIC_PREFIX
#undef BOOST_UNITS_METRIC_PREFIX
#endif
#define BOOST_UNITS_METRIC_PREFIX(exponent, name)                                                       \
    template <typename Unit> \
    struct name ## _scale { \
    typedef typename make_scaled_unit<Unit, scale<10, static_rational<exponent> > >::type type; \
    };

BOOST_UNITS_METRIC_PREFIX(-24, yocto);
BOOST_UNITS_METRIC_PREFIX(-21, zepto);
BOOST_UNITS_METRIC_PREFIX(-18, atto);
BOOST_UNITS_METRIC_PREFIX(-15, femto);
BOOST_UNITS_METRIC_PREFIX(-12, pico);
BOOST_UNITS_METRIC_PREFIX(-9, nano);
BOOST_UNITS_METRIC_PREFIX(-6, micro);
BOOST_UNITS_METRIC_PREFIX(-3, milli);
BOOST_UNITS_METRIC_PREFIX(-2, centi);
BOOST_UNITS_METRIC_PREFIX(-1, deci);
BOOST_UNITS_METRIC_PREFIX(1, deka);
BOOST_UNITS_METRIC_PREFIX(2, hecto);
BOOST_UNITS_METRIC_PREFIX(3, kilo);
BOOST_UNITS_METRIC_PREFIX(6, mega);
BOOST_UNITS_METRIC_PREFIX(9, giga);
BOOST_UNITS_METRIC_PREFIX(12, tera);
BOOST_UNITS_METRIC_PREFIX(15, peta);
BOOST_UNITS_METRIC_PREFIX(18, exa);
BOOST_UNITS_METRIC_PREFIX(21, zetta);
BOOST_UNITS_METRIC_PREFIX(24, yotta);

#undef BOOST_UNITS_METRIC_PREFIX

}
}
}

#endif
