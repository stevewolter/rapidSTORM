#ifndef NONLINFIT_TERMINATOR_H
#define NONLINFIT_TERMINATOR_H

#include <boost/concept/usage.hpp>
#include <Eigen/Core>

namespace nonlinfit {

/** \brief Concept for a fit-termination control class.
 *
 *  A class that models the Terminator concept can be used
 *  for controlling the termination of fitting algorithms.
 */
template <class X, class Position>
struct Terminator 
{
    /** This method is called for each failed computation of a fit
     *  step. The failure is typically due to a singular Hessian 
     *  matrix or numerical instability. */
    void matrix_is_unsolvable();
    /** This method is called when a fit step could be computed, but
     *  the function value did not improve with the step.
     *  \param valid_position is true if the new position is valid,
     *                        but worse than the old. */
    void failed_to_improve( bool valid_position );
    /** This method is called after a successful fit step has been taken.
     *  \param new_position The variable values after the step
     *  \param shift The step vector (size of the step for each variable)
     **/
    void improved( const Position& new_position, const Position& shift );
    /** This method should return true while fitting should continue. */
    bool should_continue_fitting() const;

    BOOST_CONCEPT_USAGE(Terminator)
    {
        X j(exemplar);            // require copy construction
        j.matrix_is_unsolvable(); // require unsolvability callback
        j.failed_to_improve(bool()); // require failed-step callback
        j.improved(pos, pos);     // require improved-step callback
        is_bool( j.should_continue_fitting() ); // required callback
    }
  private:
    X exemplar;
    const Position pos;

    void is_bool( bool b );
};

/** Check whether the template parameter is a model of Terminator. */
template <class Implementation>
bool is_terminator()
{
    BOOST_CONCEPT_ASSERT((Terminator< Implementation, Eigen::VectorXd >));
    BOOST_CONCEPT_ASSERT((Terminator< Implementation, Eigen::Vector3d >));
    return true;
};
 
}

#endif
