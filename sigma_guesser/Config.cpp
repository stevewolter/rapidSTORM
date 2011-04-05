#include "Config.h"
#include <dStorm/doc/context.h>

namespace dStorm {
namespace sigma_guesser {


Config::Config()
:   simparm::Object("EstimatePSF", "Estimate PSF size"),
    fitSigma("FitSigma", "Try to estimate PSF size", true),
    delta_sigma("DeltaSigma", "Accepted error in sigma estimation", 0.05),
    maximum_estimation_size("MaximumEstimationSize", "Limit for PSF estimation", 300000000 * camera::pixel * camera::pixel)
{
    delta_sigma.setUserLevel(Object::Expert);
    maximum_estimation_size.setUserLevel(Object::Expert);

    fitSigma.helpID = HELP_FixSigma;
    fitSigma.setUserLevel(Object::Beginner);
    fitSigma.setHelp("If this option is enabled, the program "
                     "will try to guess correct values for the "
                     "PSF size.");
}

void Config::registerNamedEntries() {
    push_back(fitSigma);
    push_back(delta_sigma);
    push_back(maximum_estimation_size);
}

}
}
