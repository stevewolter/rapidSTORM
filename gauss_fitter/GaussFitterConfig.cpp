#include "GaussFitterConfig.h"
#include "doc/help/context.h"

namespace dStorm {
namespace engine {

using namespace simparm;

GaussFitterConfig::GaussFitterConfig() 
: simparm::Set("GaussFitter", "Fit with Gaussian function"),
    sigma_xy_negligible_limit("CorrNegligibleLimit",
        "Limit up to which X-Y correlation is considered negligible", 0.1),
    marquardtStartLambda("MarquardtStartLambda",
        "Start value for Marquardt lambda factor", 1E-2),
    maximumIterationSteps("MaximumIterationSteps",
        "Maximum number of iteration steps for spot fitting", 100),
    negligibleStepLength("NegligibleStepLength", 
        "Maximum length of negligibly short iteration step", 5E-3),
    successiveNegligibleSteps("SuccessiveNegligibleSteps",
        "Number of successive negligibly short steps indicating fit "
        "success", 1),
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
    marquardtStartLambda.setUserLevel(Object::Expert);
    maximumIterationSteps.setUserLevel(Object::Intermediate);
    negligibleStepLength.setUserLevel(Object::Intermediate);
    successiveNegligibleSteps.setUserLevel(Object::Expert);

    asymmetry_threshold.helpID = HELP_AsymmetryThreshold;
    asymmetry_threshold.setHelp(
        "If spot residues are found to be more asymmetric than this "
        "value, double-spot analysis is performed. 0.1 is a good "
        "'aggressive' value here for much double-spot analysis, 1 "
        "disables the feature completely.");
    required_peak_distance.userLevel = Object::Intermediate;
    fixCorrelationTerm.userLevel = Object::Expert;
}

GaussFitterConfig::~GaussFitterConfig() {
}

void GaussFitterConfig::registerNamedEntries() 
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
