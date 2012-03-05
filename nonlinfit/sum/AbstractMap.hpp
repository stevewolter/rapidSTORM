#ifndef NONLINFIT_SUM_ABSTRACTMAP_HPP
#define NONLINFIT_SUM_ABSTRACTMAP_HPP

#include "AbstractMap.h"
#include <boost/foreach.hpp>
#include <utility>

namespace nonlinfit {
namespace sum {

template <int VarC>
void AbstractMap<VarC>::add_function( std::bitset<VarC> common )
{
    bool first_row = map.empty();
    Row r;
    for (int j = 0; j < VarC; ++j)  {
        if ( !common[j] || first_row )
            r[j] = output_var_c++;
        else
            r[j] = map[0][j];
    }
    map.push_back( r );
}

template <int VarC>
AbstractMap<VarC>::AbstractMap( int function_count, std::bitset<VarC> common )
: output_var_c(0)
{
    for (int i = 0; i < function_count; ++i)
        add_function( common );
}

template <int VarC>
template <typename Function>
void AbstractMap<VarC>::add_function( const Function& reducer_functor )
{
    BOOST_STATIC_ASSERT(( boost::is_same< std::pair<int,int>, 
                                   typename Function::result_type >::value ));
    int my_row = map.size();
    map.push_back( Row() );
    for (int j = 0; j < VarC; ++j)  {
        std::pair<int,int> reduction = reducer_functor( my_row, j );
        assert( reduction.first <= my_row && reduction.second <= j );
        if ( reduction.second < 0 )
            map[my_row][j] = -1;
        else if ( reduction.first == my_row && reduction.second == j )
            map[my_row][j] = output_var_c++;
        else
            map[my_row][j] = map[reduction.first][reduction.second];
    }
}

}
}

#endif
