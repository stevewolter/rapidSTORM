#include "calibrate_3d/FormCalibrationConfig.h"
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
    circular_psf_.setHelpID( "EstimatePSF_CircularPSF" );
    astigmatism_.setHelpID( "EstimatePSF_Astigmatism" );
    universal_best_sigma_.setHelpID( "EstimatePSF_UniversalBestSigma" );
    universal_prefactors_.setHelpID( "EstimatePSF_UniversalWidening" );
    fit_best_sigma_.setHelpID( "EstimatePSF_FitBestSigma" );
    fit_focus_plane_.setHelpID( "EstimatePSF_FitFocusPlane" );
    fit_prefactors_.setHelpID( "EstimatePSF_FitPrefactors" );

    BOOST_STATIC_ASSERT( int(sizeof(term_names) / sizeof(term_names[0])) >= polynomial_3d::Order );
    for (int i = 0; i < polynomial_3d::Order; ++i)
        z_terms[i] = boost::in_place( term_names[i] + std::string("Term"), 
            std::string("Fit ") + 
            char( tolower( term_names[i][0] ) ) + std::string( term_names[i]+1 ) +
            std::string(" 3D term"),
            false );
    (*z_terms[ polynomial_3d::offset(polynomial_3d::Quadratic)]) = true;
}

void FormCalibrationConfig::register_generic_entries( simparm::NodeHandle at )
{
    circular_psf_.attach_ui(at);
    fit_best_sigma_.attach_ui( at );
}

void FormCalibrationConfig::register_multiplane_entries( simparm::NodeHandle at )
{
    universal_best_sigma_.attach_ui(at);
    fit_prefactors_.attach_ui( at );
}

void FormCalibrationConfig::register_polynomial3d_entries( simparm::NodeHandle at )
{
    astigmatism_.attach_ui( at );
    fit_focus_plane_.attach_ui( at );
    for (int i = 0; i < polynomial_3d::Order; ++i)
        z_terms[i]->attach_ui( at );
    universal_prefactors_.attach_ui( at );
}

}
}
