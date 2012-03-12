#include "Config.h"

namespace dStorm {
namespace calibrate_3d {

Config_::Config_()
: simparm::Object("Calibrate3D", "Calibrate 3D on known data"),
  filter_("3DFilter", "Filter expression for usable spots"),
  new_z_("CalibratedZ", "Expression for true Z value"),
  target_volume_("TargetVolume", "Estimation target volume", 1E-3)
{
}

void Config_::registerNamedEntries() {
    push_back( new_z_ );
    push_back( filter_ );
    push_back( target_volume_ );
    FormCalibratorConfig::registerNamedEntries( *this );
}

}
}
