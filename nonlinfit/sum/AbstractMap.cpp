#ifndef NONLINFIT_SUM_ABSTRACTMAP_HPP
#define NONLINFIT_SUM_ABSTRACTMAP_HPP

#include <boost/foreach.hpp>
#include <utility>

namespace nonlinfit {
namespace sum {

void AbstractMap::add_function( std::vector<bool> common )
{
    bool first_row = map.empty();
    Row r(common.size());
    for (int j = 0; j < common.size(); ++j)  {
        if ( !common[j] || first_row )
            r[j] = output_var_c++;
        else
            r[j] = map[0][j];
    }
    map.push_back( r );
}

AbstractMap::AbstractMap( int function_count, std::vector<bool> common )
: map(function_count, common.size()), output_var_c(0)
{
    for (int i = 0; i < function_count; ++i) {
        add_function( common );
    }
}

}
}

#endif
