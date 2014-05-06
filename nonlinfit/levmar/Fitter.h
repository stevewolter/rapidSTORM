#ifndef NONLINFIT_LEVENBERGMARQUARDT_FITTER_H
#define NONLINFIT_LEVENBERGMARQUARDT_FITTER_H

#include "nonlinfit/AbstractFunction.h"
#include "nonlinfit/levmar/Config.h"
#include "nonlinfit/Terminator.h"

namespace nonlinfit {
namespace levmar {

/** Levenberg-Marquardt nonlinear function minimizer. */
class Fitter {
    const double initial_lambda, wrong_position_adjustment, unsolvable_adjustment,
                 pure_gradient_lambda;
    enum Step { BetterPosition, WorsePosition, InvalidPosition };
    class State;
  public:
    Fitter( const Config& );
    /** Find the minimum of the provided function with Levenberg-Marquardt.
     *  This method will repeatedly call Function::evaluate() and use the
     *  calculated parameters according to the LM method to find the minimum
     *  of the provided function. It moves the function position using the
     *  model of Moveable provided in #moveable.
     *
     *  \pre{ function is in some valid state }
     *  \post{ function is in a locally minimal state }
     *  \param[in] function     The Function model to be minimized.
     *  \param[in] terminator   A model of Terminator that controls when
     *                          fitting should stop.
     *
     *  \return The function value at the minimal position.
     **/
    double fit( AbstractFunction<double>& function, Terminator& terminator );
};

}
}

#endif
