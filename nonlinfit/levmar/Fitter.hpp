#ifndef NONLINFIT_LEVMAR_IMPL_H
#define NONLINFIT_LEVMAR_IMPL_H

#include "debug.h"
#include "Fitter.h"
#include "exceptions.h"

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
namespace levmar {

/** Helper class for Fitter that stores the current fitting state. */
template <typename _Function, typename _Moveable>
class Fitter::State {
    typedef _Function Function;
    typedef typename _Moveable::Position Position;
    typedef typename Function::Derivatives Derivatives;

    struct Parameters {
        Position parameters;
        Derivatives derivatives;
        Parameters( int n ) : parameters(n), derivatives(n) {}
    };
    
    Parameters max, moritz, *work, *trial;
    typename Derivatives::Vector original_hessians_diagonal;
    typename Derivatives::Matrix scratch;

    Position shift;

    const bool use_ldlt;

    typedef typename Derivatives::Vector Shift;

  public:
    /** Retrieve the position of function as the current position. */
    State( Function& function, const _Moveable& m, int number_of_variables );
    /** Try to compute a step to the next location based on the current
     *  position and the LM lambda parameter. 
     *  \return Success of solving the equation system. */
    bool solve_equations( double lambda );
    const Position& current_position() const { return work->parameters; }
    /** After try_to_move_position() returned Fitter::BetterPosition,
     *  this method can be used to obtain the difference between the
     *  previous and the current position. */
    const Position& last_shift() const { return shift; }
    double current_function_value() const { return work->derivatives.value; }
    /** Try to shift the current position by the previously computed step.
     *  The function is evaluated at the new position to determine its
     *  validity.
     *  \pre{ solve_equations() has returned true }
     **/
    Step try_to_move_position( _Function&, _Moveable& );
    void throw_singular_matrix() const 
        { throw SingularMatrix( work->derivatives.hessian.template cast<double>() ); }
 
  private:
    bool solve_by_inversion( const Derivatives& );
    bool solve_with_ldlt( const Derivatives& );
    bool solve_with_llt( const Derivatives& );
};

template <typename _Function, typename _Moveable>
Fitter::State<_Function,_Moveable>::State( _Function& f, const _Moveable& m, int n )
: max(n), moritz(n) , work( &max ), trial( &moritz ),
  original_hessians_diagonal( n ), scratch(n,n), shift(n),
  use_ldlt( n > 4 )
{
    BOOST_STATIC_ASSERT(( _Function::Derivatives::VariableCount == Eigen::Dynamic 
                       || _Function::Derivatives::VariableCount > 0 ));
    m.get_position( work->parameters );
    bool initial_position_valid = f.evaluate( work->derivatives );
    if ( ! initial_position_valid )
        throw InvalidStartPosition(work->parameters.template cast<double>());
    original_hessians_diagonal = work->derivatives.hessian.diagonal();
    DEBUG("Started fitting at " << work->parameters.transpose() << " with value " << work->derivatives.value);
}

template <typename _Function, typename _Moveable>
bool Fitter::State<_Function,_Moveable>::solve_equations( double lambda )
{
    assert( ! work->derivatives.contains_NaN() );
    assert( original_hessians_diagonal == original_hessians_diagonal );

    work->derivatives.hessian.diagonal() = 
        original_hessians_diagonal * ( 1 + lambda);
    DEBUG("Solving equations for " << work->derivatives.gradient.transpose() << " and " << work->derivatives.hessian.diagonal().transpose() );
    return solve_with_ldlt( work->derivatives )
        || solve_with_llt( work->derivatives );
}

template <typename _Function, typename _Moveable>
bool Fitter::State<_Function,_Moveable>::solve_by_inversion( const Derivatives& d )
{
    double determinant;
    bool invertible;
    d.hessian.computeInverseAndDetWithCheck(
        scratch, determinant, invertible, 1E-30 );
    if ( invertible ) {
        shift = scratch * d.gradient;
        DEBUG("Solved system by inversion");
        return true;
    } else {
        DEBUG("Failed to solve system by inversion");
        return false;
    }
}

template <typename _Function, typename _Moveable>
bool Fitter::State<_Function,_Moveable>::solve_with_ldlt( const Derivatives& d )
{
    if ( boost::is_same< float, typename Derivatives::Vector::Scalar >::value )
    {
        shift = ( d.hessian.template cast<double>().ldlt()
                .solve(d.gradient.template cast<double>()) )
                .template cast<typename Derivatives::Vector::Scalar>();
    } else {
        shift = d.hessian.ldlt().solve(d.gradient);
    }
    Position gradient_check = (d.hessian * shift) - d.gradient;
    /* This code is very similar to Eigen's isApprox; however, with gcc 4.7 and Eigen 3.1,
     * isApprox generates segmentation faults. */
    const bool solved_well = gradient_check.squaredNorm() <= 0.1 * d.gradient.squaredNorm();
    DEBUG("Solved system by LDLT: " << solved_well);
    return solved_well;
}

template <typename _Function, typename _Moveable>
bool Fitter::State<_Function,_Moveable>::solve_with_llt( const Derivatives& d )
{
    if ( boost::is_same< float, typename Derivatives::Vector::Scalar >::value )
    {
        shift = ( d.hessian.template cast<double>().llt()
                .solve(d.gradient.template cast<double>()) )
                .template cast<typename Derivatives::Vector::Scalar>();
    } else {
        shift = d.hessian.llt().solve(d.gradient);
    }
    bool solved_well = (d.hessian * shift).isApprox(d.gradient, 1E-2);
    DEBUG("Solved system by LLT: " << solved_well);
    return solved_well;
}

template <typename _Function, typename _Moveable>
Fitter::Step
Fitter::State<_Function,_Moveable>::try_to_move_position( _Function& f, _Moveable& m )
{
    trial->parameters = work->parameters + shift;
    DEBUG("Evaluating function at " << trial->parameters.transpose() << " after step of " << shift.transpose() );
    m.set_position( trial->parameters );
    /* Compute the function at this place. */
    const bool new_position_valid = f.evaluate( trial->derivatives);

    if ( ! new_position_valid ) return InvalidPosition;

    assert( ! trial->derivatives.contains_NaN() );

    DEBUG("Position is valid, value changed from " << work->derivatives.value << " to " << trial->derivatives.value);
    const bool new_position_better = 
        trial->derivatives.value <= work->derivatives.value;
    if ( ! new_position_better ) return WorsePosition;

    std::swap( work, trial );
    original_hessians_diagonal = work->derivatives.hessian.diagonal();
    return BetterPosition;
}

template <typename _Function, typename _Moveable, typename _Terminator>
double Fitter::fit( _Function& f, _Moveable& m, _Terminator t )
{
    State<_Function,_Moveable> state(f, m, f.variable_count());
    double lambda = initial_lambda;
    do {
        const bool solvable_equations = state.solve_equations( lambda );
        if ( ! solvable_equations ) {
            if ( lambda > pure_gradient_lambda ) state.throw_singular_matrix();
            t.matrix_is_unsolvable();
            lambda *= unsolvable_adjustment;
            continue;
        } else {
            Step result = state.try_to_move_position( f, m );
            if ( result == BetterPosition ) {
                t.improved( state.current_position(), state.last_shift() );
                lambda /= wrong_position_adjustment;
                DEBUG("The shift of " << state.last_shift().transpose() << " improved the function, lambda is " << lambda );
            } else if ( result == WorsePosition || result == InvalidPosition ) {
                lambda *= wrong_position_adjustment;
                t.failed_to_improve( result == WorsePosition );
                DEBUG("The new position is " << ((result == WorsePosition) ? "worse" : "invalid") << " and lambda is " << lambda);
            } else { assert( false ); }
        }
    } while ( t.should_continue_fitting() );
    DEBUG("Finished fitting at " << state.current_position().transpose());

    m.set_position( state.current_position() );
    return state.current_function_value();
}

}
}

#endif
