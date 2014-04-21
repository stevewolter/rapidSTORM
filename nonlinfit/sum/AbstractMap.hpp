#include "nonlinfit/sum/AbstractMap.h"
#include <utility>

namespace nonlinfit {
namespace sum {

template <typename Function>
void AbstractMap::add_function( int variable_count, const Function& reducer_functor )
{
    BOOST_STATIC_ASSERT(( boost::is_same< std::pair<int,int>, 
                                   typename Function::result_type >::value ));
    int my_row = map.size();
    map.push_back( std::vector<int>(variable_count) );
    for (int j = 0; j < variable_count; ++j)  {
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
