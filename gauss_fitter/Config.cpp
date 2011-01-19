#include "Config.h"
#include <dStorm/doc/context.h>
#include "fitter/MarquardtConfig_impl.h"
#include "fitter/residue_analysis/Config_impl.h"

namespace dStorm {
namespace gauss_2d_fitter {

using namespace simparm;

Config::Config() 
: MarquardtConfig("2DFitter", "Fit with 2D Gaussian function"),
    sigma_xy_negligible_limit("CorrNegligibleLimit",
        "Limit up to which X-Y correlation is considered negligible", 0.1),
    freeSigmaFitting("FreeSigmaFitting", "Fit with free covariance matrix",
                     false),
    fixCorrelationTerm("FixCorrelationTerm", "Do not fit correlation term")
{
    freeSigmaFitting.helpID = HELP_FreeForm;

    sigma_xy_negligible_limit.setUserLevel(Object::Intermediate);

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
    fitter::residue_analysis::Config::registerNamedEntries(*this);
}

}
}
