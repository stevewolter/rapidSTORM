#include "Config.h"

namespace dStorm {
namespace estimate_psf_form {

Config::Config()
:   simparm::Object("FitPSFForm", "Estimate PSF form"),
    multiplane( "MultiPlane", "Multi-layer specific options"),
    polynomial_3d( "Polynomial", "Polynomial 3D specific options"),
    mle("FormMLE", "Use MLE to fit PSF form", false), 
    number_of_spots("EstimationSpots", "Number of spots used in estimation", 40),
    max_per_image("MaxEstimationSpotsPerImage", "Number of spots used per image", 15.0),
    visual_selection("SelectSpots", "Manually select good spots", true),
    laempi_fit("LaempiPosition", "Laempi fit for positions", false),
    disjoint_amplitudes("LaempiAmplitudes", "Disjoint amplitude fit", false),
    z_is_truth("ZIsTruth", "Z position is ground truth", false),
    fit_window_width("FitWindowWidth", "Fit window radius", FitWindowWidth::Constant(600 * si::nanometre) )
{
}

void Config::registerNamedEntries() {
    multiplane.push_back( laempi_fit );
    multiplane.push_back( disjoint_amplitudes );
    FormCalibrationConfig::register_multiplane_entries( multiplane );

    polynomial_3d.push_back( z_is_truth );
    FormCalibrationConfig::register_polynomial3d_entries( polynomial_3d );

    push_back( number_of_spots );
    push_back( max_per_image );
    push_back( visual_selection );
    push_back( mle );
    FormCalibrationConfig::register_generic_entries( *this );

    push_back( multiplane );
    push_back( polynomial_3d );

}

}
}
