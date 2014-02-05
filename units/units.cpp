#include "units/nanoresolution.h"
#include "units/megafrequency.h"
#include "units/nanolength.h"
#include "units/microlength.h"
#include "units/permicrolength.h"

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

std::string name_string(const si::nanolength&) {
    return "nanometer";
}
std::string symbol_string(const si::nanolength&)
{
    return "nm";
}

std::string name_string(const si::permicrolength&) {
    return "micrometer^-1";
}
std::string symbol_string(const si::permicrolength&)
{
    return "µm^-1";
}


}
}
