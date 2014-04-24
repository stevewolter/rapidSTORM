#ifndef NONLINFIT_SUM_ABSTRACTFUNCTION_HPP
#define NONLINFIT_SUM_ABSTRACTFUNCTION_HPP

#include "nonlinfit/sum/AbstractFunction.h"
#include "debug.h"
#include <cmath>

namespace nonlinfit {
namespace sum {

AbstractFunction::AbstractFunction( const VariableMap& variable_map )
: fitters( variable_map.function_count(), static_cast<argument_type*>(NULL) ), 
  map( variable_map ),
  plane_count(variable_map.function_count()),
  position_buffer(variable_map.input_var_c),
  new_position_buffer(variable_map.input_var_c),
  evaluation_buffer(variable_map.input_var_c),
  variables_are_dropped(variable_map.variables_are_dropped()) {}

void AbstractFunction::get_position( Position& p ) const
{
    assert( p.rows() == variable_count() );
#ifndef NDEBUG
    p.fill( std::numeric_limits< typename Position::Scalar >::signaling_NaN() );
#endif
    for (int i = 0; i < plane_count; ++i) {
        assert( fitters[i] );
        fitters[i]->get_position( position_buffer );
        DEBUG("Getting parameters " << position_buffer.transpose() << " from upstream #" << i);
        for (int j = 0; j < position_buffer.rows(); ++j) {
            int downstream_pos = map(i,j);
            if ( downstream_pos >= 0 )
            {
                typename Position::Scalar& mapped_variable = p[ downstream_pos ];
                DEBUG("Downstream variable of upstream " << i << " and variable " << j << " is at " 
                    << map(i,j) << " and is changed from " << mapped_variable
                    << " to " << position_buffer[j]);
                assert( std::isnan( mapped_variable ) || std::abs( mapped_variable - position_buffer[j] ) < 1E-50 );
                mapped_variable = position_buffer[j];
            }
        }
    }
}

void AbstractFunction::set_position( const Position& p ) 
{
    for ( int i = 0; i < plane_count; ++i )
    {
        assert( fitters[i] );
        /* When variables have been dropped, we have to get their default
         * values first. */
        if ( variables_are_dropped )
            fitters[i]->get_position( position_buffer );
        for (int r = 0; r < position_buffer.rows(); ++r) {
            const int row = map(i,r);
            if ( row >= 0 )
                position_buffer[r] = p[ map(i,r) ];
        }
        DEBUG("Setting parameters " << position_buffer.transpose() << " for upstream #" << i);
        fitters[i]->set_position( position_buffer );
    }
}

bool AbstractFunction::evaluate( Derivatives& p )
{
    assert( p.hessian.rows() == variable_count() );
    assert( p.hessian.cols() == variable_count() );
    assert( p.gradient.rows() == variable_count() );

    p.set_zero();

    for ( int i = 0; i < plane_count; ++i )
    {
        assert( fitters[i] );
        DEBUG("Evaluating upstream #" << i);
        if ( ! fitters[i]->evaluate( evaluation_buffer ) )
            return false;
        assert( ! evaluation_buffer.contains_NaN() );
        p.value += evaluation_buffer.value;
        for (int r = 0; r < map.input_var_c; ++r) {
            int tr = map(i,r);
            if ( tr >= 0 ) {
                p.gradient[ tr ] += evaluation_buffer.gradient[r];
                for (int c = 0; c < map.input_var_c; ++c) {
                    const int tc = map(i,c);
                    if ( tc >= 0 )
                        p.hessian(tr, tc) += evaluation_buffer.hessian(r,c);
                }
            }
        }
    }
    return true;
}

bool AbstractFunction::step_is_negligible(
        const Position& old_position, const Position& new_position ) const {
    for ( int i = 0; i < plane_count; ++i ) {
        assert( fitters[i] );
        /* When variables have been dropped, we have to get their default
         * values first. */
        if ( variables_are_dropped ) {
            fitters[i]->get_position( position_buffer );
            fitters[i]->get_position( new_position_buffer );
        }
        for (int r = 0; r < position_buffer.rows(); ++r) {
            const int row = map(i,r);
            if ( row >= 0 ) {
                position_buffer[r] = old_position[ map(i,r) ];
                new_position_buffer[r] = new_position[ map(i,r) ];
            }
        }
        DEBUG(" " << position_buffer.transpose() << " for upstream #" << i);
        if (!fitters[i]->step_is_negligible( position_buffer, new_position_buffer )) {
            return false;
        }
    }
    return true;
}

}
}

#endif
