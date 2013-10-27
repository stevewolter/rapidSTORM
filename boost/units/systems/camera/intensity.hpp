#ifndef CSUNITS_CAMERA_INTENSITY_HPP
#define CSUNITS_CAMERA_INTENSITY_HPP

#include "base.hpp"
#include <boost/units/static_constant.hpp>

namespace boost {
namespace units {
namespace camera {

typedef intensity_base_unit::unit_type intensity;

BOOST_UNITS_STATIC_CONSTANT(ad_count,intensity);
BOOST_UNITS_STATIC_CONSTANT(ad_counts,intensity);

}
}
}

#endif
