#include <cassert>
#include <limits>

#include <Eigen/Cholesky> 
#include <Eigen/LU> 

#include <boost/type_traits/is_same.hpp>
#include <boost/static_assert.hpp>
#include <boost/test/unit_test.hpp>

#include "nonlinfit/functions/Polynom.h"
#include "nonlinfit/VectorPosition.h"
#include "nonlinfit/terminators/StepLimit.h"
#include "nonlinfit/levmar/Fitter.h"
#include "nonlinfit/levmar/exceptions.h"
#include "nonlinfit/InvalidPositionError.h"

#include "debug.h"

#ifdef VERBOSE
#include <iomanip>
#include <iostream>
#endif

/** \cond */

namespace nonlinfit {
namespace levmar {

/** Helper class for Fitter that stores the current fitting state. */
class Fitter::State {
    typedef AbstractFunction<double> Function;
    typedef Function::Derivatives Derivatives;
    typedef Function::Position Position;

    struct Parameters {
        Position parameters;
        Derivatives derivatives;
        Parameters( int n ) : parameters(n), derivatives(n) {}
    };
    
    Parameters max, moritz, *work, *trial;
    Derivatives::Vector original_hessians_diagonal;
    Derivatives::Matrix scratch;

    Position shift;

    const bool use_ldlt;

  public:
    /** Retrieve the position of function as the current position. */
    State( Function& function, int number_of_variables );
    /** Try to compute a step to the next location based on the current
     *  position and the LM lambda parameter. 
     *  \return Success of solving the equation system. */
    bool solve_equations( double lambda );
    const Position& current_position() const { return work->parameters; }
    /** After try_to_move_position() returned Fitter::BetterPosition,
     *  this method can be used to obtain the previous position. */
    const Position& previous_position() const { return trial->parameters; }
    double current_function_value() const { return work->derivatives.value; }
    /** Try to shift the current position by the previously computed step.
     *  The function is evaluated at the new position to determine its
     *  validity.
     *  \pre{ solve_equations() has returned true }
     **/
    Step try_to_move_position( AbstractFunction<double>& );
    void throw_singular_matrix() const 
        { throw SingularMatrix( work->derivatives.hessian.cast<double>() ); }
 
  private:
    bool solve_with_ldlt( const Derivatives& );
    bool solve_with_llt( const Derivatives& );
};

Fitter::State::State( AbstractFunction<double>& f, int n )
: max(n), moritz(n) , work( &max ), trial( &moritz ),
  original_hessians_diagonal( n ), scratch(n,n), shift(n),
  use_ldlt( n > 4 )
{
    f.get_position( work->parameters );
    bool initial_position_valid = false;
    try {
        initial_position_valid = f.evaluate( work->derivatives );
    } catch (const InvalidPositionError&) {}
    if ( ! initial_position_valid )
        throw InvalidStartPosition(work->parameters.cast<double>());
    original_hessians_diagonal = work->derivatives.hessian.diagonal();
    DEBUG("Started fitting at " << work->parameters.transpose() << " with value " << work->derivatives.value);
}

bool Fitter::State::solve_equations( double lambda )
{
    assert( ! work->derivatives.contains_NaN() );
    assert( original_hessians_diagonal == original_hessians_diagonal );

    work->derivatives.hessian.diagonal() = 
        original_hessians_diagonal * ( 1 + lambda);
    DEBUG("Solving equations for " << work->derivatives.gradient.transpose() << " and " << work->derivatives.hessian.diagonal().transpose() );
    return solve_with_ldlt( work->derivatives )
        || solve_with_llt( work->derivatives );
}

bool Fitter::State::solve_with_ldlt( const Derivatives& d )
{
    if ( boost::is_same< float, Derivatives::Vector::Scalar >::value )
    {
        shift = ( d.hessian.cast<double>().ldlt()
                .solve(d.gradient.cast<double>()) )
                .cast<Derivatives::Vector::Scalar>();
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

bool Fitter::State::solve_with_llt( const Derivatives& d )
{
    if ( boost::is_same< float, Derivatives::Vector::Scalar >::value )
    {
        shift = ( d.hessian.cast<double>().llt()
                .solve(d.gradient.cast<double>()) )
                .cast<Derivatives::Vector::Scalar>();
    } else {
        shift = d.hessian.llt().solve(d.gradient);
    }
    bool solved_well = (d.hessian * shift).isApprox(d.gradient, 1E-2);
    DEBUG("Solved system by LLT: " << solved_well);
    return solved_well;
}

Fitter::Step
Fitter::State::try_to_move_position( Function& f )
{
    trial->parameters = work->parameters + shift;
    DEBUG("Evaluating function at " << trial->parameters.transpose() << " after step of " << shift.transpose() );
    f.set_position( trial->parameters );
    /* Compute the function at this place. */
    try {
        if ( ! f.evaluate(trial->derivatives) ) {
            return InvalidPosition;
        }
    } catch (const InvalidPositionError& e) {
        return InvalidPosition;
    }

    assert( ! trial->derivatives.contains_NaN() );

    DEBUG("Position is valid, value changed from " << work->derivatives.value << " to " << trial->derivatives.value);
    const bool new_position_better = 
        trial->derivatives.value <= work->derivatives.value;
    if ( ! new_position_better ) return WorsePosition;

    std::swap( work, trial );
    original_hessians_diagonal = work->derivatives.hessian.diagonal();
    return BetterPosition;
}

Fitter::Fitter( const Config& config ) 
: initial_lambda( config.initial_lambda ),
  wrong_position_adjustment( 10 ),
  unsolvable_adjustment( 1.1 ),
  pure_gradient_lambda( 1E20 )
{}

double Fitter::fit( AbstractFunction<double>& f, Terminator& t )
{
    State state(f, f.variable_count());
    double lambda = initial_lambda;
    do {
        const bool solvable_equations = state.solve_equations( lambda );
        if ( ! solvable_equations ) {
            if ( lambda > pure_gradient_lambda ) state.throw_singular_matrix();
            t.matrix_is_unsolvable();
            lambda *= unsolvable_adjustment;
            continue;
        } else {
            Step result = state.try_to_move_position( f );
            if ( result == BetterPosition ) {
                t.improved( f.step_is_negligible(state.previous_position(), state.current_position()) );
                lambda /= wrong_position_adjustment;
                DEBUG("The shift from " << state.previous_position() << " to " << state.current_position().transpose() << " improved the function, lambda is " << lambda );
            } else if ( result == WorsePosition || result == InvalidPosition ) {
                lambda *= wrong_position_adjustment;
                t.failed_to_improve( result == WorsePosition );
                DEBUG("The new position is " << ((result == WorsePosition) ? "worse" : "invalid") << " and lambda is " << lambda);
            } else { assert( false ); }
        }
    } while ( t.should_continue_fitting() );
    DEBUG("Finished fitting at " << state.current_position().transpose());

    f.set_position( state.current_position() );
    return state.current_function_value();
}



static void check_naive_fitter_finds_paraboles_minimum() {
    typedef nonlinfit::Bind< static_power::Expression, static_power::BaseValue > Lambda;
    Lambda a;
    static_power::SimpleFunction<0,1> function(a);

    a( static_power::Variable() ) = 5;
    a( static_power::Power() ) = 2;
    a( static_power::Prefactor() ) = 2;

    Config config;
    Fitter fitter(config);
    terminators::StepLimit terminator(150);
    fitter.fit( function, terminator );
    BOOST_CHECK_SMALL( a( static_power::Variable() ), 0.01 );
}

boost::unit_test::test_suite* register_unit_tests() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "levmar" );
    rv->add( BOOST_TEST_CASE( &check_naive_fitter_finds_paraboles_minimum ) );
    return rv;
}

}
}

/** \endcond */
