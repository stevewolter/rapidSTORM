#include "MarquardtConfig.h"

namespace dStorm {
namespace fitter {

MarquardtConfig::MarquardtConfig(std::string name, std::string desc)
: simparm::Set(name, desc),
    marquardtStartLambda("MarquardtStartLambda",
        "Start value for Marquardt lambda factor", 1E-2),
    maximumIterationSteps("MaximumIterationSteps",
        "Maximum number of iteration steps for spot fitting", 100),
    negligibleStepLength("NegligibleStepLength", 
        "Maximum length of negligibly short iteration step", 5E-3),
    successiveNegligibleSteps("SuccessiveNegligibleSteps",
        "Number of successive negligibly short steps indicating fit "
        "success", 1)
{
    marquardtStartLambda.setUserLevel(Object::Expert);
    maximumIterationSteps.setUserLevel(Object::Intermediate);
    negligibleStepLength.setUserLevel(Object::Intermediate);
    successiveNegligibleSteps.setUserLevel(Object::Expert);
}

MarquardtConfig::~MarquardtConfig() {}

void MarquardtConfig::registerNamedEntries()
{
}

}
}
