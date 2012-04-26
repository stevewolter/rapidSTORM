#include "FormCalibrationConfig.h"
#include <boost/static_assert.hpp>

namespace dStorm {
namespace calibrate_3d {

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

FormCalibrationConfig::FormCalibrationConfig()
: circular_psf_("CircularPSF", "Assume circular PSF", true),
  astigmatism_("Astigmatism", "Allow astigmatism", false),
  universal_best_sigma_("UniversalBestSigma", "PSF FWHM common to all layers", false ),
  universal_prefactors_("UniversalWidening", "3D widening common to all layers", false ),
  fit_best_sigma_( "FitBestSigma", "Fit PSF FWHM", true ),
  fit_focus_plane_( "FitFocusPlane", "Fit focus plane Z coordinate", false ),
  fit_prefactors_( "FitPrefactors", "Fit transmission factors", true )
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

void FormCalibrationConfig::register_generic_entries( simparm::Node& at )
{
    at.push_back( circular_psf_ );
    at.push_back( fit_best_sigma_ );
}

void FormCalibrationConfig::register_multiplane_entries( simparm::Node& at )
{
    at.push_back( universal_best_sigma_ );
    at.push_back( fit_prefactors_ );
}

void FormCalibrationConfig::register_polynomial3d_entries( simparm::Node& at )
{
    at.push_back( astigmatism_ );
    at.push_back( fit_focus_plane_ );
    for (int i = 0; i < polynomial_3d::Order; ++i)
        at.push_back( *z_terms[i] );
    at.push_back( universal_prefactors_ );
}

}
}
