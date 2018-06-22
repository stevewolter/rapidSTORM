#ifndef NONLINFIT_SUM_ABSTRACTFUNCTION_HPP
#define NONLINFIT_SUM_ABSTRACTFUNCTION_HPP

#include "nonlinfit/sum/AbstractFunction.h"
#include "debug.h"
#include <cmath>

namespace nonlinfit {
namespace sum {

template <typename Number, typename Policy>
AbstractFunction<Number, Policy>::AbstractFunction( const VariableMap& variable_map )
: fitters( variable_map.function_count(), static_cast<argument_type*>(NULL) ), 
  movers( variable_map.function_count(), static_cast<moveable_type*>(NULL) ), 
  map( variable_map ),
  plane_count(variable_map.function_count()),
  position_buffer(variable_map.input_var_c),
  evaluation_buffer(variable_map.input_var_c) {}

template <typename Number, typename Policy>
void AbstractFunction<Number, Policy>::get_position( Position& p ) const
{
    assert( p.rows() == variable_count() );
#ifndef NDEBUG
    p.fill( std::numeric_limits< typename Position::Scalar >::signaling_NaN() );
#endif
    for (int i = 0; i < plane_count; ++i) {
        assert( movers[i] );
        movers[i]->get_position( position_buffer );
        DEBUG("Getting parameters " << position_buffer.transpose() << " from upstream #" << i);
        for (int j = 0; j < position_buffer.rows(); ++j) {
            int downstream_pos = map(i,j);
            if ( ! Policy::VariablesAreDropped || downstream_pos >= 0 )
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

template <typename Number, typename Policy>
void AbstractFunction<Number, Policy>::set_position( const Position& p ) 
{
    for ( int i = 0; i < plane_count; ++i )
    {
        assert( movers[i] );
        /* When variables have been dropped, we have to get their default
         * values first. */
        if ( Policy::VariablesAreDropped )
            movers[i]->get_position( position_buffer );
        for (int r = 0; r < position_buffer.rows(); ++r) {
            const int row = map(i,r);
            if ( ! Policy::VariablesAreDropped || row >= 0 )
                position_buffer[r] = p[ map(i,r) ];
        }
        DEBUG("Setting parameters " << position_buffer.transpose() << " for upstream #" << i);
        movers[i]->set_position( position_buffer );
    }
}

template <typename Number, typename Policy>
bool AbstractFunction<Number, Policy>::evaluate( Derivatives& p )
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
            if ( ! Policy::VariablesAreDropped || tr >= 0 ) {
                p.gradient[ tr ] += evaluation_buffer.gradient[r];
                for (int c = 0; c < map.input_var_c; ++c) {
                    const int tc = map(i,c);
                    if ( ! Policy::VariablesAreDropped || tc >= 0 )
                        p.hessian(tr, tc) += evaluation_buffer.hessian(r,c);
                }
            }
        }
    }
    return true;
}

}
}

#endif
