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

FormCalibratorConfig::FormCalibratorConfig()
: circular_psf_("CircularPSF", "Assume circular PSF", true),
  astigmatism_("Astigmatism", "Allow astigmatism", false),
  universal_best_sigma_("UniversalBestSigma", "PSF FWHM common to all layers", false ),
  fit_best_sigma_( "FitBestSigma", "Fit PSF FWHM", true ),
  fit_focus_plane_( "FitFocusPlane", "Fit focus plane Z coordinate", true )
{
    BOOST_STATIC_ASSERT( int(sizeof(term_names) / sizeof(term_names[0])) >= polynomial_3d::Order );
    for (int i = 0; i < polynomial_3d::Order; ++i)
        z_terms[i] = boost::in_place( term_names[i] + std::string("Term"), 
            std::string("Fit ") + 
            char( tolower( term_names[i][0] ) ) + std::string( term_names[i]+1 ) +
            std::string(" 3D term"),
            false );
    (*z_terms[ polynomial_3d::offset(polynomial_3d::Quadratic)]) = true;
}

void FormCalibratorConfig::registerNamedEntries( simparm::Node& at )
{
    at.push_back( circular_psf_ );
    at.push_back( astigmatism_ );
    at.push_back( universal_best_sigma_ );
    at.push_back( fit_best_sigma_ );
    at.push_back( fit_focus_plane_ );
    for (int i = 0; i < polynomial_3d::Order; ++i)
        at.push_back( *z_terms[i] );
}

Config::Config()
:   simparm::Object("FitPSFForm", "Estimate PSF form"),
    auto_disable("AutoDisable", "Raise no error for missing source images", false), 
    mle("FormMLE", "Use MLE to fit PSF form", false), 
    number_of_spots("EstimationSpots", "Number of spots used in estimation", 40),
    max_per_image("MaxEstimationSpotsPerImage", "Number of spots used per image", 1),
    visual_selection("SelectSpots", "Manually select good spots", true),
    laempi_fit("LaempiPosition", "Laempi fit for positions", false),
    disjoint_amplitudes("LaempiAmplitudes", "Disjoint amplitude fit", false),
    z_calibration("ZCalibrationFile", "Z calibration file")
{
    auto_disable.userLevel = simparm::Object::Intermediate;
}

void Config::registerNamedEntries() {
    push_back( auto_disable );
    push_back( mle );
    push_back( number_of_spots );
    push_back( max_per_image );
    push_back( visual_selection );
    push_back( laempi_fit );
    push_back( disjoint_amplitudes );
    push_back( z_calibration );
    FormCalibratorConfig::registerNamedEntries( *this );
}

}
}
