#include "Config.h"
#include "doc/help/context.h"

namespace dStorm {
namespace 2d_fitter {

using namespace simparm;

Config::Config() 
: MarquardtConfig("2DFitter", "Fit with 2D Gaussian function"),
    sigma_xy_negligible_limit("CorrNegligibleLimit",
        "Limit up to which X-Y correlation is considered negligible", 0.1),
    asymmetry_threshold("AsymmetryThreshold", 
                        "Threshold for relative spot asymmetry", 1),
    required_peak_distance("RequiredPeakDistance",
                        "Minimum distance between double-spot peaks in nm", 250),
    freeSigmaFitting("FreeSigmaFitting", "Fit with free covariance matrix",
                     false),
    fixCorrelationTerm("FixCorrelationTerm", "Do not fit correlation term")
{
    freeSigmaFitting.helpID = HELP_FreeForm;

    sigma_xy_negligible_limit.setUserLevel(Object::Intermediate);

    asymmetry_threshold.helpID = HELP_AsymmetryThreshold;
    asymmetry_threshold.setHelp(
        "If spot residues are found to be more asymmetric than this "
        "value, double-spot analysis is performed. 0.1 is a good "
        "'aggressive' value here for much double-spot analysis, 1 "
        "disables the feature completely.");
    required_peak_distance.userLevel = Object::Intermediate;
    fixCorrelationTerm.userLevel = Object::Expert;
}

Config::~Config() {
}

void Config::registerNamedEntries() 
{
    push_back(sigma_xy_negligible_limit);
    push_back(freeSigmaFitting);
    push_back(fixCorrelationTerm);
    push_back(marquardtStartLambda);
    push_back(maximumIterationSteps);
    push_back(negligibleStepLength);
    push_back(successiveNegligibleSteps);
    push_back(asymmetry_threshold);
    push_back(required_peak_distance);
}

}
}
