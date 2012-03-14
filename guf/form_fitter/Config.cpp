#include "Config.h"

namespace dStorm {
namespace form_fitter {

Config::Config()
:   simparm::Object("FitPSFForm", "Estimate PSF form"),
    auto_disable("AutoDisable", "Raise no error for missing source images", false), 
    mle("FormMLE", "Use MLE to fit PSF form", false), 
    number_of_spots("EstimationSpots", "Number of spots used in estimation", 40),
    width_correction("WidthCorrection", "Correction factor for PSF width", 1.075),
    max_per_image("MaxEstimationSpotsPerImage", "Number of spots used per image", 1.0),
    visual_selection("SelectSpots", "Manually select good spots", true),
    laempi_fit("LaempiPosition", "Laempi fit for positions", false),
    disjoint_amplitudes("LaempiAmplitudes", "Disjoint amplitude fit", false)
{
    auto_disable.userLevel = simparm::Object::Intermediate;
}

void Config::registerNamedEntries() {
    push_back( auto_disable );
    push_back( mle );
    push_back( number_of_spots );
    push_back( max_per_image );
    push_back( width_correction );
    push_back( visual_selection );
    push_back( laempi_fit );
    push_back( disjoint_amplitudes );
    FormCalibrationConfig::registerNamedEntries( *this );
}

}
}
