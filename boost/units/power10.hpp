#ifndef CSUNITS_CAMERA_POWER10_H
#define CSUNITS_CAMERA_POWER10_H

#include <boost/units/make_scaled_unit.hpp>

namespace boost {
namespace units {

/** This metafunction scales a unit type by a power of 10.
 *  E.g. power10<meters,3>::type gives kilometers. */
template <typename Unit, int Power>
struct power10 {
    typedef typename make_scaled_unit< Unit, scale<10, static_rational<Power> > >::type type;
};

}
}

#endif
