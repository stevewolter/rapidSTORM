#include "calibrate_3d/Config.h"

namespace dStorm {
namespace calibrate_3d {

Config::Config()
: target_volume_("TargetVolume", "Estimation target volume", 1E-3),
  target_localization_number_("TargetLocalizationNumber", "Correct number of localizations", 100),
  missing_penalty_("MissingPenalty", "Penalty for missing localizations", 2 * si::micrometer),
  relative_initial_step_("RelativeInitialStep", "Length of initial step relative to initial position", 0.01),
  absolute_initial_step_("AbsoluteInitialStep", "Length of initial step", 1e-3)
{
}

void Config::attach_ui( simparm::NodeHandle at ) {
    target_volume_.attach_ui( at );
    target_localization_number_.attach_ui( at );
    missing_penalty_.attach_ui( at );
    relative_initial_step_.attach_ui( at );
    absolute_initial_step_.attach_ui( at );
    FormCalibrationConfig::register_generic_entries( at );
    FormCalibrationConfig::register_multiplane_entries( at );
    FormCalibrationConfig::register_polynomial3d_entries( at );
    ZTruthConfig::registerNamedEntries( at );
}

}
}
