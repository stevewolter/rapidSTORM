#include "FormCalibrationConfig.h"
#include <boost/static_assert.hpp>
#include "ZTruth.h"

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
  fit_best_sigma_( "FitBestSigma", "Fit PSF FWHM", true ),
  fit_focus_plane_( "FitFocusPlane", "Fit focus plane Z coordinate", false ),
  fit_prefactors_( "FitPrefactors", "Fit transmission factors", true ),
  filter_("3DFilter", "Filter expression for usable spots"),
  new_z_("CalibratedZ", "Expression for true Z value")
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

void FormCalibrationConfig::registerNamedEntries( simparm::Node& at )
{
    at.push_back( circular_psf_ );
    at.push_back( astigmatism_ );
    at.push_back( universal_best_sigma_ );
    at.push_back( fit_best_sigma_ );
    at.push_back( fit_focus_plane_ );
    for (int i = 0; i < polynomial_3d::Order; ++i)
        at.push_back( *z_terms[i] );
    at.push_back( new_z_ );
    at.push_back( filter_ );
}

bool FormCalibrationConfig::has_z_truth() const {
    return new_z_() != "";
}
std::auto_ptr<ZTruth> FormCalibrationConfig::get_z_truth() const {
    if ( has_z_truth() )
        return std::auto_ptr<ZTruth>( new ZTruth( filter_(), new_z_() ) );
    else
        throw std::runtime_error("A Z calibration expression is necessary, but not given");
}

}
}
