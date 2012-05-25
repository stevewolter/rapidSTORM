#include <simparm/BoostUnits.h>
#include "UnitEntries.h"
#include "UnitEntries/Hertz.h"
#include "UnitEntries/FrameEntry.h"
#include "UnitEntries/TemperatureEntry.h"
#include "UnitEntries/TimeEntry.h"
#include "UnitEntries/Nanometre.h"
#include "UnitEntries/PixelSize.h"
#include <boost/units/systems/si/io.hpp>
#include <boost/units/systems/camera/resolution.hpp>

#include <simparm/Entry.hpp>
#include <simparm/Attribute.hpp>
#include <dStorm/units/microlength.h>
#include <dStorm/units/permicrolength.h>
#include <dStorm/units/camera_response.h>

namespace simparm {
using namespace boost::units;
using boost::optional;
using Eigen::Matrix;

template class Entry< quantity< si::nanolength, double > >;
template class Entry< quantity< si::microlength, double > >;
template class Entry< quantity< si::nanolength, float > >;
template class Entry< quantity< si::length, float > >;
template class Entry< quantity< si::microtime, float > >;
template class Entry< quantity< si::time, float > >;
template class Entry< quantity< si::time, int > >;
template class Entry< quantity< si::frequency, float > >;
template class Entry< quantity< si::megafrequency, float > >;
template class Entry< quantity< celsius::temperature, float > >;
template class Entry< quantity< celsius::temperature, int > >;
template class Entry< quantity< camera::length, int > >;
template class Entry< quantity< camera::length, float > >;
template class Entry< quantity< camera::area, int > >;
template class Entry< quantity< camera::intensity, float > >;
template class Entry< quantity< camera::intensity, int > >;
template class Entry< quantity< camera::time, float > >;
template class Entry< quantity< camera::time, int > >;
template class Entry< quantity< camera::resolution, float > >;
template class Entry< quantity< dStorm::nanometer_pixel_size, float > >;
template class Entry< quantity< dStorm::nanoresolution, float > >;
template class Entry< optional< quantity< camera::intensity, float > > >;
template class Entry< optional< quantity< camera::intensity, double > > >;
template class Entry< optional< quantity< camera::intensity, int > > >;
template class Entry< optional< quantity< camera::time, int > > >;

template class Entry< quantity< boost::units::divide_typeof_helper< camera::time, camera::length >::type, int > >;
template class Entry< quantity< boost::units::divide_typeof_helper< si::dimensionless, camera::length >::type, float > >;
template class Entry< quantity< boost::units::divide_typeof_helper< si::dimensionless, camera::length >::type, double > >;
template class Entry< quantity< boost::units::divide_typeof_helper< si::dimensionless, camera::length >::type, int > >;
template class Entry< quantity< boost::units::divide_typeof_helper< camera::intensity, camera::length >::type, float > >;

template class Entry< quantity< boost::units::divide_typeof_helper< 
    boost::units::power_typeof_helper< si::length, boost::units::static_rational<2> >::type,
    camera::time >::type > >;
template class Entry< quantity< boost::units::divide_typeof_helper< 
    boost::units::power_typeof_helper< si::length, boost::units::static_rational<2> >::type,
    boost::units::power_typeof_helper< camera::time, boost::units::static_rational<3> >::type
>::type > >;

template class Entry< Matrix< quantity< si::nanolength, double >, 2, 1, Eigen::DontAlign > >;
template class Entry< Matrix< quantity< si::nanolength, double >, 3, 1, Eigen::DontAlign > >;
template class Entry< Matrix< quantity< camera::length, double >, 2, 1, Eigen::DontAlign > >;
template class Entry< Matrix< quantity< camera::length, int >, 2, 1, Eigen::DontAlign > >;
template class Entry< Matrix< quantity< camera::length, int >, 3, 1 > >;
template class Entry< optional< Matrix< quantity< divide_typeof_helper< power10<si::length, -12>::type, camera::time >::type, float >, 3, 1, Eigen::DontAlign > > >;
template class Entry< Matrix< quantity< dStorm::nanometer_pixel_size, float >, 2, 1, Eigen::DontAlign > >;
template class Entry< Matrix< quantity< si::permicrolength, double >, 2, 4, Eigen::DontAlign > >;
}
