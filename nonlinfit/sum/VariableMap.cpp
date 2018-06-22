#ifndef NONLINFIT_SUM_ABSTRACTMAP_HPP
#define NONLINFIT_SUM_ABSTRACTMAP_HPP

#include "nonlinfit/sum/VariableMap.h"
#include <boost/foreach.hpp>
#include <utility>

namespace nonlinfit {
namespace sum {

VariableMap::VariableMap( int function_count, std::vector<bool> common )
: output_var_c(0)
{
    for (int i = 0; i < function_count; ++i) {
        add_function( common );
    }
}

void VariableMap::add_function( std::vector<bool> common )
{
    bool first_row = map.empty();
    Row r(common.size());
    for (size_t j = 0; j < common.size(); ++j)  {
        if ( !common[j] || first_row )
            r[j] = output_var_c++;
        else
            r[j] = map[0][j];
    }
    map.push_back( r );
}

}
}

#endif
