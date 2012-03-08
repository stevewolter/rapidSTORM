#include "Config.h"
#include <boost/static_assert.hpp>

namespace dStorm {
namespace form_fitter {

static const char *term_names[] = {
    "Linear",
    "Quadratic",
    "Cubic",
    "Quartic",
    "Quintic",
    "Sextic",
    "Septic",
    "Octic",
    "Nonic"
};

Config::Config()
:   simparm::Object("FitPSFForm", "Estimate PSF form"),
    auto_disable("AutoDisable", "Raise no error for missing source images", false), 
    mle("FormMLE", "Use MLE to fit PSF form", false), 
    number_of_spots("EstimationSpots", "Number of spots used in estimation", 40),
    max_per_image("MaxEstimationSpotsPerImage", "Number of spots used per image", 1),
    visual_selection("SelectSpots", "Manually select good spots", true),
    circular_psf("CircularPSF", "Assume circular PSF", true),
    laempi_fit("LaempiPosition", "Laempi fit for positions", false),
    disjoint_amplitudes("LaempiAmplitudes", "Disjoint amplitude fit", false),
    z_calibration("ZCalibrationFile", "Z calibration file")
{
    auto_disable.userLevel = simparm::Object::Intermediate;
    BOOST_STATIC_ASSERT( int(sizeof(term_names) / sizeof(term_names[0])) >= polynomial_3d::Order );
    for (int i = 0; i < polynomial_3d::Order; ++i)
        z_terms[i] = boost::in_place( term_names[i] + std::string("Term"), 
            std::string("Fit ") + 
            char( tolower( term_names[i][0] ) ) + std::string( term_names[i]+1 ) +
            std::string("of polynomial 3D") );
    (*z_terms[1]) = true;
}

void Config::registerNamedEntries() {
    push_back( auto_disable );
    push_back( mle );
    push_back( number_of_spots );
    push_back( max_per_image );
    push_back( visual_selection );
    push_back( circular_psf );
    push_back( laempi_fit );
    push_back( disjoint_amplitudes );
    for (int i = 0; i < polynomial_3d::Order; ++i)
        push_back( *z_terms[i] );
    push_back( z_calibration );
}

}
}
