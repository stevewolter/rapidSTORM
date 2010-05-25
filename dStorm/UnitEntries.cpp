#include "UnitEntries.h"
#include <simparm/UnitEntry_Impl.hh>

namespace simparm {
template class simparm::UnitEntry< boost::units::si::nanolength, double >;
template class simparm::UnitEntry< cs_units::camera::length, int >;
template class simparm::UnitEntry< cs_units::camera::length, float >;
template class simparm::UnitEntry< cs_units::camera::intensity, float >;
}
