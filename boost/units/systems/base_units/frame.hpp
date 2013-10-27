#ifndef CSUNITS_BASE_UNITS_FRAME_HPP
#define CSUNITS_BASE_UNITS_FRAME_HPP

#include <boost/units/physical_dimensions/time.hpp>

namespace boost {
namespace units {

struct frame_base_unit 
    : public boost::units::base_unit<frame_base_unit, 
                    boost::units::time_dimension, -101> 
    {};

template<> 
struct base_unit_info<frame_base_unit>
{
    static std::string name() { return "frame"; }
    static std::string symbol() { return "fr"; }
};

}
}

#endif
