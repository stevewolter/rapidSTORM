#ifndef NONLINFIT_LEVENBERGMARQUARDT_CONFIG_H
#define NONLINFIT_LEVENBERGMARQUARDT_CONFIG_H

#include "nonlinfit/levmar/fwd.h"

namespace nonlinfit {
namespace levmar {

/** This class contains the parameters for Levenberg-Marquardt fitters. */
struct Config
{
    /** The start value for Fitter::lambda */
    double initial_lambda;
    Config() : initial_lambda(1) {}
};

}
}

#endif
