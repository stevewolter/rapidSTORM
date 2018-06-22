#include "estimate_psf_form/Config.h"

namespace dStorm {
namespace estimate_psf_form {

Config::Config()
:   multiplane( "MultiPlane", "Multi-layer specific options"),
    mle("FormMLE", "Use MLE to fit PSF form", false), 
    number_of_spots("EstimationSpots", "Number of spots used in estimation", 40),
    max_per_image("MaxEstimationSpotsPerImage", "Number of spots used per image", 15.0),
    visual_selection("SelectSpots", "Manually select good spots", true),
    laempi_fit("LaempiPosition", "Laempi fit for positions", false),
    disjoint_amplitudes("LaempiAmplitudes", "Disjoint amplitude fit", false)
{
}

void Config::attach_ui( simparm::NodeHandle at ) {
    simparm::NodeHandle m = multiplane.attach_ui( at );
    laempi_fit.attach_ui( m );
    disjoint_amplitudes.attach_ui( m );
    FormCalibrationConfig::register_multiplane_entries( m );

    number_of_spots.attach_ui( at );
    max_per_image.attach_ui( at );
    visual_selection.attach_ui( at );
    mle.attach_ui( at );
    FormCalibrationConfig::register_generic_entries( at );

    fit_window_config.attach_ui(at);
}

}
}
