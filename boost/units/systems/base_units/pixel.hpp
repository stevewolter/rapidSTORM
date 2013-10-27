#ifndef CSUNITS_BASE_UNITS_PIXEL_HPP
#define CSUNITS_BASE_UNITS_PIXEL_HPP

#include <boost/units/base_unit.hpp>
#include <boost/units/physical_dimensions/length.hpp>

namespace boost {
namespace units {

struct pixel_base_unit 
    : public boost::units::base_unit<pixel_base_unit, 
                    boost::units::length_dimension, -100> 
    {};

template<> 
struct base_unit_info<pixel_base_unit>
{
    static std::string name() { return "pixel"; }
    static std::string symbol() { return "px"; }
};

}
}

#endif
