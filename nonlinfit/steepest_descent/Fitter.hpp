#ifndef NONLINFIT_STEEPEST_DESCENT_HPP
#define NONLINFIT_STEEPEST_DESCENT_HPP

#include "debug.h"

#include <cassert>
#include <limits>

#include <Eigen/Cholesky> 
#include <Eigen/LU> 
#include <boost/type_traits/is_same.hpp>
#include <boost/static_assert.hpp>

#ifdef VERBOSE
#include <iomanip>
#include <iostream>
#endif

namespace nonlinfit {
namespace steepest_descent {

/** Levenberg-Marquardt nonlinear function minimizer. */
class Fitter {
    const double initial_step_size, wrong_position_adjustment, unsolvable_adjustment;
    enum Step { BetterPosition, WorsePosition, InvalidPosition };
  public:
    inline Fitter();
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
    template <typename Function_, typename Moveable_, typename _Terminator>
    double fit( Function_& function, Moveable_& moveable, _Terminator terminator );
};

Fitter::Fitter() 
: initial_step_size( 1 ),
  wrong_position_adjustment( 2 ),
  unsolvable_adjustment( 1.1 )
{}

template <typename Function_, typename Moveable_, typename _Terminator>
double Fitter::fit( Function_& function, Moveable_& moveable, _Terminator terminator )
{
    typename Moveable_::Position best_position, trial_position;
    typename Function_::Derivatives best_evaluation, trial_evaluation;

    moveable.get_position( best_position );
    bool initial_position_is_good = function.evaluate( best_evaluation );
    if ( ! initial_position_is_good )
        throw std::runtime_error("Invalid initial fit position");

    DEBUG("Initial function value is " << best_evaluation.value);
    double step_size = initial_step_size;
    do {
        typename Moveable_::Position step = - step_size * best_evaluation.gradient.array();
        trial_position = best_position + step;
        moveable.set_position( trial_position );
        DEBUG("Evaluating at trial position " << trial_position.transpose());
        bool valid_position = function.evaluate( trial_evaluation );
        if ( valid_position && trial_evaluation.value < best_evaluation.value ) {
            best_position = trial_position;
            best_evaluation = trial_evaluation;
            terminator.improved( trial_position, step );
            step_size *= wrong_position_adjustment;
            DEBUG("The shift of " << step.transpose() << " improved the function to " << best_evaluation.value << ", step size is " << step_size );
        } else if ( valid_position ) {
            step_size /= wrong_position_adjustment;
            terminator.failed_to_improve( true );
            DEBUG("The new position is worse at value " << trial_evaluation.value << " and step size is " << step_size);
        } else {
            step_size /= wrong_position_adjustment;
            terminator.failed_to_improve( false );
            DEBUG("The new position is invalid and step size is " << step_size);
        }
    } while ( terminator.should_continue_fitting() );
    DEBUG("Finished fitting at " << best_position.transpose());

    moveable.set_position( best_position );
    return best_evaluation.value;
}

}
}

#endif
