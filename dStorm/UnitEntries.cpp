#include "UnitEntries.h"
#include "UnitEntries/Hertz.h"
#include "UnitEntries/FrameEntry.h"
#include "UnitEntries/TemperatureEntry.h"
#include "UnitEntries/TimeEntry.h"
#include "UnitEntries/Nanometre.h"
#include "UnitEntries/PixelSize.h"
#include <simparm/UnitEntry_Impl.hh>
#include <boost/units/systems/si/io.hpp>

namespace simparm {
template class simparm::UnitEntry< boost::units::si::nanolength, double >;
template class simparm::UnitEntry< boost::units::si::nanolength, float >;
template class simparm::UnitEntry< boost::units::si::microtime, float >;
template class simparm::UnitEntry< boost::units::si::time, float >;
template class simparm::UnitEntry< boost::units::si::time, int >;
template class simparm::UnitEntry< boost::units::si::frequency, float >;
template class simparm::UnitEntry< boost::units::si::megafrequency, float >;
template class simparm::UnitEntry< boost::units::celsius::temperature, float >;
template class simparm::UnitEntry< boost::units::celsius::temperature, int >;
template class simparm::UnitEntry< cs_units::camera::length, int >;
template class simparm::UnitEntry< cs_units::camera::length, float >;
template class simparm::UnitEntry< cs_units::camera::intensity, float >;
template class simparm::UnitEntry< cs_units::camera::time, float >;
template class simparm::UnitEntry< cs_units::camera::time, int >;
template class simparm::UnitEntry< dStorm::nanometer_pixel_size, float >;
template class simparm::UnitEntry< dStorm::nanoresolution, float >;
}
