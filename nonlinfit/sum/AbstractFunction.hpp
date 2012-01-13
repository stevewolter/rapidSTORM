#ifndef NONLINFIT_SUM_ABSTRACTFUNCTION_HPP
#define NONLINFIT_SUM_ABSTRACTFUNCTION_HPP

#include "AbstractFunction.h"
#include "debug.h"
#include <cmath>

namespace nonlinfit {
namespace sum {

template <typename F, typename M, int PC, int MPC, int MOVC>
AbstractFunction<F,M,PC,MPC,MOVC>::AbstractFunction( const VariableMap& variable_map )
: fitters( variable_map.function_count(), static_cast<argument_type*>(NULL) ), 
  movers( variable_map.function_count(), static_cast<moveable_type*>(NULL) ), 
  map( variable_map ),
    plane_count(variable_map.function_count()) 
{ 
    assert( variable_count() <= MOVC || MOVC == Eigen::Dynamic );
}

template <typename F, typename M, int PC, int MPC, int MOVC>
void AbstractFunction<F,M,PC,MPC,MOVC>::get_position( Position& p ) const
{
    assert( p.rows() == variable_count() );
#ifndef NDEBUG
    p.fill( std::numeric_limits< typename Position::Scalar >::signaling_NaN() );
#endif
    typename moveable_type::Position onepos;
    for (int i = 0; i < plane_count; ++i) {
        assert( movers[i] );
        movers[i]->get_position( onepos );
        for (int j = 0; j < InputVarC; ++j) {
            typename Position::Scalar& mapped_variable = p[ map(i,j) ];
            assert( std::isnan( mapped_variable ) || std::abs( mapped_variable - onepos[j] ) < 1E-50 );
            mapped_variable = onepos[j];
        }
    }
}

template <typename F, typename M, int PC, int MPC, int MOVC>
void AbstractFunction<F,M,PC,MPC,MOVC>::set_position( const Position& p ) 
{
    typename moveable_type::Position upstream;
    for ( int i = 0; i < plane_count; ++i )
    {
        assert( movers[i] );
        for (int r = 0; r < InputVarC; ++r)
            upstream[r] = p[ map(i,r) ];
        movers[i]->set_position( upstream );
    }
}

template <typename F, typename M, int PC, int MPC, int MOVC>
bool AbstractFunction<F,M,PC,MPC,MOVC>::evaluate( Derivatives& p )
{
    typename argument_type::Derivatives upstream;
    assert( p.hessian.rows() == variable_count() );
    assert( p.hessian.cols() == variable_count() );
    assert( p.gradient.rows() == variable_count() );

    p.set_zero();

    for ( int i = 0; i < plane_count; ++i )
    {
        assert( fitters[i] );
        if ( ! fitters[i]->evaluate( upstream ) )
            return false;
        assert( ! upstream.contains_NaN() );
        p.value += upstream.value;
        for (int r = 0; r < InputVarC; ++r) {
            int tr = map(i,r);
            p.gradient[ tr ] += upstream.gradient[r];
            for (int c = 0; c < InputVarC; ++c) {
                p.hessian(tr, map(i,c)) += upstream.hessian(r,c);
            }
        }
    }
    return true;
}

}
}

#endif
