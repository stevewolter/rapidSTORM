#include "nanoresolution.h"
#include "megafrequency.h"

namespace boost {
namespace units {

std::string name_string(const dStorm::nanoresolution&) {
    return "pixel per nanometer";
}
std::string symbol_string(const dStorm::nanoresolution&)
{
    return "px/nm";
}

std::string name_string(const dStorm::nanometer_pixel_size&) {
    return "nanometer per pixel";
}
std::string symbol_string(const dStorm::nanometer_pixel_size&)
{
    return "nm/px";
}


}
}
