#ifndef NONLINFIT_SUM_ABSTRACTMAP_H
#define NONLINFIT_SUM_ABSTRACTMAP_H

#include <Eigen/Core>
#include <bitset>
#include <vector>
#include <boost/array.hpp>

namespace nonlinfit {
namespace sum {

/** Mapping from parameters on multiple functions to a single parameter sequence.
 *  \tparam VariableCount The number of variables for each input function.
 **/
template < int VariableCount >
struct AbstractMap {
    typedef  boost::array<int,VariableCount> Row;
    std::vector<Row> map;
    int output_var_c;

  public:
    AbstractMap() : output_var_c(0) {}
    /** Construct by calling add_function() function_count times. */
    AbstractMap(int function_count, std::bitset<VariableCount> common );
    /** Add a function that has all bitset-indicated variables in common with
     *  the first function. */
    void add_function( std::bitset<VariableCount> common );
    /** Add a function with variable commonness indicated by reduction functor.
     * A reduction functor is supplied with two arguments, the function index
     * and a parameter index, and returns a std::pair<int,int> indicating a
     * function and parameter that is the same value. If it returns its 
     * arguments, a new output parameter is created for the current parameter.
     **/
    template <typename Function>
    void add_function( const Function& reducer_functor );

    /** Get the output parameter index for the given input function index and
     *  variable. */
    int operator()( int function, int variable ) const { return map[function][variable]; }
    int function_count() const { return map.size(); }
    /** Get the number of variables in the target sequence. */
    int output_variable_count() const { return output_var_c; }
};

}
}

#endif
