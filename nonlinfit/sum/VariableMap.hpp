#ifndef NONLINFIT_SUM_ABSTRACTMAP_HPP
#define NONLINFIT_SUM_ABSTRACTMAP_HPP

#include "nonlinfit/sum/VariableMap.h"
#include <boost/foreach.hpp>
#include <utility>

namespace nonlinfit {
namespace sum {

template <typename Function>
void VariableMap::add_function(const Function& reducer_functor )
{
    BOOST_STATIC_ASSERT(( boost::is_same< std::pair<int,int>, 
                          typename Function::result_type >::value ));
    int my_row = map.size();
    map.push_back( Row(input_var_c) );
    for (int j = 0; j < input_var_c; ++j)  {
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
