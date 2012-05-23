#include "Config.h"
#include <simparm/Entry_Impl.hh>

namespace dStorm {
namespace calibrate_3d {

Config::Config()
: target_volume_("TargetVolume", "Estimation target volume", 1E-3),
  target_localization_number_("TargetLocalizationNumber", "Correct number of localizations", 100),
  missing_penalty_("MissingPenalty", "Penalty for missing localizations", 2 * si::micrometer)
{
}

void Config::attach_ui( simparm::NodeHandle at ) {
    target_volume_.attach_ui( at );
    target_localization_number_.attach_ui( at );
    missing_penalty_.attach_ui( at );
    FormCalibrationConfig::register_generic_entries( at );
    FormCalibrationConfig::register_multiplane_entries( at );
    FormCalibrationConfig::register_polynomial3d_entries( at );
    ZTruthConfig::registerNamedEntries( at );
}

}
}
