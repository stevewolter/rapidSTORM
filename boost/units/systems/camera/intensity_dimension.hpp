#ifndef CSUNITS_CAMERA_INTENSITY_DIMENSION_H
#define CSUNITS_CAMERA_INTENSITY_DIMENSION_H

#include <boost/units/base_dimension.hpp>
#include <boost/units/physical_dimensions/luminous_intensity.hpp>

namespace boost {
namespace units {
namespace camera {

struct intensity_base_unit 
    : public base_unit<intensity_base_unit, 
                    luminous_intensity_dimension, 1> 
{};

}

template<> 
struct base_unit_info<camera::intensity_base_unit>
{
    static std::string name() { return "A/D count"; }
    static std::string symbol() { return "ADC"; }
};

}
}

#endif
