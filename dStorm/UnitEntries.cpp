#include <simparm/BoostUnits.h>
#include "UnitEntries.h"
#include "UnitEntries/Hertz.h"
#include "UnitEntries/FrameEntry.h"
#include "UnitEntries/TemperatureEntry.h"
#include "UnitEntries/TimeEntry.h"
#include "UnitEntries/Nanometre.h"
#include "UnitEntries/PixelSize.h"
#include <simparm/Entry_Impl.h>
#include <boost/units/systems/si/io.hpp>
#include <boost/units/systems/camera/resolution.hpp>

namespace simparm {
using namespace boost::units;
template class Entry< boost::units::quantity< si::nanolength, double > >;
template class Entry< boost::units::quantity< si::nanolength, float > >;
template class Entry< boost::units::quantity< si::microtime, float > >;
template class Entry< boost::units::quantity< si::time, float > >;
template class Entry< boost::units::quantity< si::time, int > >;
template class Entry< boost::units::quantity< si::frequency, float > >;
template class Entry< boost::units::quantity< si::megafrequency, float > >;
template class Entry< boost::units::quantity< celsius::temperature, float > >;
template class Entry< boost::units::quantity< celsius::temperature, int > >;
template class Entry< boost::units::quantity< camera::length, int > >;
template class Entry< boost::units::quantity< camera::length, float > >;
template class Entry< boost::units::quantity< camera::area, int > >;
template class Entry< boost::units::quantity< camera::intensity, float > >;
template class Entry< boost::units::quantity< camera::time, float > >;
template class Entry< boost::units::quantity< camera::time, int > >;
template class Entry< boost::units::quantity< camera::resolution, float > >;
template class Entry< boost::units::quantity< dStorm::nanometer_pixel_size, float > >;
template class Entry< boost::units::quantity< dStorm::nanoresolution, float > >;
template class Entry< boost::optional< boost::units::quantity< camera::intensity, float > > >;
}
