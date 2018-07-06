#include "simparm/BoostUnits.h"
#include "UnitEntries.h"
#include "UnitEntries/Hertz.h"
#include "UnitEntries/FrameEntry.h"
#include "UnitEntries/TemperatureEntry.h"
#include "UnitEntries/TimeEntry.h"
#include "UnitEntries/Nanometre.h"
#include "UnitEntries/PixelSize.h"
#include <boost/units/systems/si/io.hpp>
#include "boost/units/systems/camera/resolution.hpp"

#include "simparm/Entry.hpp"
#include "simparm/Attribute.hpp"
#include "units/microlength.h"
#include "units/permicrolength.h"
#include "units/camera_response.h"

namespace simparm {
using namespace boost::units;
using boost::optional;
using Eigen::Matrix;

#define INSTANTIATE(...) \
    template class Entry< __VA_ARGS__ >; \
    template bool Attribute< boost::optional<__VA_ARGS__> >::valueChange(const boost::optional<__VA_ARGS__>&, bool); \
    template bool Attribute< __VA_ARGS__ >::valueChange(const __VA_ARGS__&, bool);
#define INSTANTIATE_NO_OPTIONAL(...) \
    template class Entry< __VA_ARGS__ >; \
    template bool Attribute< __VA_ARGS__ >::valueChange(const __VA_ARGS__&, bool);


INSTANTIATE( quantity< si::nanolength, double > )
INSTANTIATE( quantity< si::microlength, double > )
INSTANTIATE( quantity< si::nanolength, float > )
INSTANTIATE( quantity< si::length, float > )
INSTANTIATE( quantity< si::microtime, float > )
INSTANTIATE( quantity< si::time, float > )
INSTANTIATE( quantity< si::time, int > )
INSTANTIATE( quantity< si::frequency, float > )
INSTANTIATE( quantity< si::megafrequency, float > )
INSTANTIATE( quantity< celsius::temperature, float > )
INSTANTIATE( quantity< celsius::temperature, int > )
INSTANTIATE( quantity< camera::length, int > )
INSTANTIATE( quantity< camera::length, float > )
INSTANTIATE( quantity< camera::area, int > )
INSTANTIATE( quantity< camera::time, float > )
INSTANTIATE( quantity< camera::resolution, float > )
INSTANTIATE( quantity< dStorm::nanometer_pixel_size, float > )
INSTANTIATE( quantity< dStorm::nanoresolution, float > )

INSTANTIATE_NO_OPTIONAL( quantity< camera::intensity, float > )
INSTANTIATE_NO_OPTIONAL( quantity< camera::intensity, int > )
INSTANTIATE_NO_OPTIONAL( quantity< camera::time, int > )
INSTANTIATE_NO_OPTIONAL( optional< quantity< camera::intensity, float > > )
INSTANTIATE_NO_OPTIONAL( optional< quantity< camera::intensity, double > > )
INSTANTIATE_NO_OPTIONAL( optional< quantity< camera::intensity, int > > )
INSTANTIATE_NO_OPTIONAL( optional< quantity< camera::time, int > > )

INSTANTIATE( quantity< boost::units::divide_typeof_helper< camera::time, camera::length >::type, int > )
INSTANTIATE( quantity< boost::units::divide_typeof_helper< si::dimensionless, camera::length >::type, float > )
INSTANTIATE( quantity< boost::units::divide_typeof_helper< si::dimensionless, camera::length >::type, double > )
INSTANTIATE( quantity< boost::units::divide_typeof_helper< si::dimensionless, camera::length >::type, int > )
INSTANTIATE( quantity< boost::units::divide_typeof_helper< camera::intensity, camera::length >::type, float > )

INSTANTIATE( quantity< boost::units::divide_typeof_helper< 
    boost::units::power_typeof_helper< si::length, boost::units::static_rational<2> >::type,
    camera::time >::type > );
INSTANTIATE( quantity< boost::units::divide_typeof_helper< 
    boost::units::power_typeof_helper< si::length, boost::units::static_rational<2> >::type,
    boost::units::power_typeof_helper< camera::time, boost::units::static_rational<3> >::type
>::type > );

INSTANTIATE( Matrix< quantity< si::nanolength, double >, 2, 1, Eigen::DontAlign > )
INSTANTIATE( Matrix< quantity< si::nanolength, double >, 3, 1, Eigen::DontAlign > )
INSTANTIATE( Matrix< quantity< si::microlength, float >, 3, 1, Eigen::DontAlign > )
INSTANTIATE( Matrix< quantity< camera::length, double >, 2, 1, Eigen::DontAlign > )
INSTANTIATE( Matrix< quantity< camera::length, int >, 2, 1, Eigen::DontAlign > )
INSTANTIATE( Matrix< quantity< camera::length, int >, 3, 1 > )
INSTANTIATE_NO_OPTIONAL( optional< Matrix< quantity< divide_typeof_helper< power10<si::length, -12>::type, camera::time >::type, float >, 3, 1, Eigen::DontAlign > > )
INSTANTIATE( Matrix< quantity< dStorm::nanometer_pixel_size, float >, 2, 1, Eigen::DontAlign > )
INSTANTIATE( Matrix< quantity< si::permicrolength, double >, 2, 4, Eigen::DontAlign > )
}
