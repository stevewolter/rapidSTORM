#ifndef NONLINFIT_LEVENBERGMARQUARDT_FITTER_H
#define NONLINFIT_LEVENBERGMARQUARDT_FITTER_H

#include "nonlinfit/levmar/fwd.h"
#include "nonlinfit/levmar/Config.h"

namespace nonlinfit {
namespace levmar {

/** Levenberg-Marquardt nonlinear function minimizer. */
class Fitter {
    const double initial_lambda, wrong_position_adjustment, unsolvable_adjustment,
                 pure_gradient_lambda;
    enum Step { BetterPosition, WorsePosition, InvalidPosition };
    template <class Function_, class Moveable_> class State;
  public:
    inline Fitter( const Config& );
    /** Find the minimum of the provided function with Levenberg-Marquardt.
     *  This method will repeatedly call Function::evaluate() and use the
     *  calculated parameters according to the LM method to find the minimum
     *  of the provided function. It moves the function position using the
     *  model of Moveable provided in #moveable.
     *
     *  \pre{ function is in some valid state }
     *  \post{ function is in a locally minimal state }
     *  \param[in] function     The Function model to be minimized.
     *  \param[in,out] moveable A Moveable model that is used to change the
     *                          position of #function in the state space.
     *  \param[in] terminator   A model of Terminator that controls when
     *                          fitting should stop.
     *
     *  \return The function value at the minimal position.
     **/
    template <typename Number>
    double fit( AbstractFunction<Number>& function, AbstractMoveable<Number>& moveable, AbstractTerminator<Number> terminator );
};

Fitter::Fitter( const Config& config ) 
: initial_lambda( config.initial_lambda ),
  wrong_position_adjustment( 10 ),
  unsolvable_adjustment( 1.1 ),
  pure_gradient_lambda( 1E20 )
{}

}
}

#endif
