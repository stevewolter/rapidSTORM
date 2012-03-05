#ifndef NONLINFIT_SUM_ABSTRACTFUNCTION_HPP
#define NONLINFIT_SUM_ABSTRACTFUNCTION_HPP

#include "AbstractFunction.h"
#include "debug.h"
#include <cmath>

namespace nonlinfit {
namespace sum {

template <typename F, typename M, typename Policy>
AbstractFunction<F,M,Policy>::AbstractFunction( const VariableMap& variable_map )
: fitters( variable_map.function_count(), static_cast<argument_type*>(NULL) ), 
  movers( variable_map.function_count(), static_cast<moveable_type*>(NULL) ), 
  map( variable_map ),
    plane_count(variable_map.function_count()) 
{ 
    assert( variable_count() <= OutputVariableCountMax ||
            OutputVariableCountMax == Eigen::Dynamic );
}

template <typename F, typename M, typename Policy>
void AbstractFunction<F,M,Policy>::get_position( Position& p ) const
{
    assert( p.rows() == variable_count() );
#ifndef NDEBUG
    p.fill( std::numeric_limits< typename Position::Scalar >::signaling_NaN() );
#endif
    typename moveable_type::Position upstream;
    for (int i = 0; i < plane_count; ++i) {
        assert( movers[i] );
        movers[i]->get_position( upstream );
        DEBUG("Getting parameters " << upstream.transpose() << " from upstream #" << i);
        for (int j = 0; j < InputVarC; ++j) {
            int downstream_pos = map(i,j);
            if ( ! Policy::VariablesAreDropped || downstream_pos >= 0 )
            {
                typename Position::Scalar& mapped_variable = p[ downstream_pos ];
                DEBUG("Downstream variable of upstream " << i << " and variable " << j << " is at " 
                    << map(i,j) << " and is changed from " << mapped_variable
                    << " to " << upstream[j]);
                assert( std::isnan( mapped_variable ) || std::abs( mapped_variable - upstream[j] ) < 1E-50 );
                mapped_variable = upstream[j];
            }
        }
    }
}

template <typename F, typename M, typename Policy>
void AbstractFunction<F,M,Policy>::set_position( const Position& p ) 
{
    typename moveable_type::Position upstream;
    for ( int i = 0; i < plane_count; ++i )
    {
        assert( movers[i] );
        /* When variables have been dropped, we have to get their default
         * values first. */
        if ( Policy::VariablesAreDropped )
            movers[i]->get_position( upstream );
        for (int r = 0; r < InputVarC; ++r) {
            const int row = map(i,r);
            if ( ! Policy::VariablesAreDropped || row >= 0 )
                upstream[r] = p[ map(i,r) ];
        }
        DEBUG("Setting parameters " << upstream.transpose() << " for upstream #" << i);
        movers[i]->set_position( upstream );
    }
}

template <typename F, typename M, typename Policy>
bool AbstractFunction<F,M,Policy>::evaluate( Derivatives& p )
{
    typename argument_type::Derivatives upstream;
    assert( p.hessian.rows() == variable_count() );
    assert( p.hessian.cols() == variable_count() );
    assert( p.gradient.rows() == variable_count() );

    p.set_zero();

    for ( int i = 0; i < plane_count; ++i )
    {
        assert( fitters[i] );
        DEBUG("Evaluating upstream #" << i);
        if ( ! fitters[i]->evaluate( upstream ) )
            return false;
        assert( ! upstream.contains_NaN() );
        p.value += upstream.value;
        for (int r = 0; r < InputVarC; ++r) {
            int tr = map(i,r);
            if ( ! Policy::VariablesAreDropped || tr >= 0 ) {
                p.gradient[ tr ] += upstream.gradient[r];
                for (int c = 0; c < InputVarC; ++c) {
                    const int tc = map(i,c);
                    if ( ! Policy::VariablesAreDropped || tc >= 0 )
                        p.hessian(tr, tc) += upstream.hessian(r,c);
                }
            }
        }
    }
    return true;
}

}
}

#endif
