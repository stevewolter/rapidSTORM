#include "Config.h"
#include "doc/help/context.h"
#include "fitter/MarquardtConfig_impl.h"
#include "fitter/residue_analysis/Config_impl.h"

namespace simparm {
template class simparm::UnitEntry<cs_units::camera::resolution, float>;
}

namespace dStorm {
namespace gauss_3d_fitter {

using namespace simparm;

Config::Config() 
: MarquardtConfig("3DFitter", "3D cylinder lens fit"),
  z_distance("ZDistance", "Distance between X and Y foci"),
  defocus_constant_x("XDefocusConstant", "Speed of PSF std. dev. growth in X"),
  defocus_constant_y("YDefocusConstant", "Speed of PSF std. dev. growth in Y"),
  holtzer_psf("HoltzerPSF", "Use Holtzerian PSF", false)
{
}

Config::~Config() {
}

void Config::registerNamedEntries() 
{
    push_back(marquardtStartLambda);
    push_back(maximumIterationSteps);
    push_back(negligibleStepLength);
    push_back(successiveNegligibleSteps);
    push_back(z_distance);
    push_back(defocus_constant_x);
    push_back(defocus_constant_y);
    push_back(holtzer_psf);
    fitter::residue_analysis::Config::registerNamedEntries(*this);
}

}
}
