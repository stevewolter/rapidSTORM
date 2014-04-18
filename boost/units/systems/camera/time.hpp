#ifndef CSUNITS_CAMERA_FRAME_HPP
#define CSUNITS_CAMERA_FRAME_HPP

#include "boost/units/systems/camera/base.hpp"
#include <boost/units/static_constant.hpp>

namespace boost {
namespace units {
namespace camera {

typedef frame_base_unit::unit_type time;

BOOST_UNITS_STATIC_CONSTANT(frame,time);
BOOST_UNITS_STATIC_CONSTANT(frames,time);

}

inline std::string name_string( const camera::time& )
    { return "frame"; }
inline std::string symbol_string( const camera::time& )
    { return "fr"; }

}
}

#endif
