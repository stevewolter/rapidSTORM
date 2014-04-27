#ifndef NONLINFIT_SUM_ABSTRACTMAP_HPP
#define NONLINFIT_SUM_ABSTRACTMAP_HPP

#include "nonlinfit/sum/VariableMap.h"
#include <boost/foreach.hpp>
#include <utility>

namespace nonlinfit {
namespace sum {

VariableMap::VariableMap( int function_count, std::vector<bool> common )
: input_var_c(common.size()), output_var_c(0)
{
    for (int i = 0; i < function_count; ++i) {
        add_function( common );
    }
}

void VariableMap::add_function( std::vector<bool> common )
{
    if (map.empty()) {
	input_var_c = common.size();
    } else {
	assert(int(common.size()) == input_var_c);
    }
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

bool VariableMap::variables_are_dropped() const {
    for (const Row& row : map) {
        for (const int& entry : row) {
            if (entry < 0) {
                return true;
            }
        }
    }
    return false;
}

}
}

#endif
