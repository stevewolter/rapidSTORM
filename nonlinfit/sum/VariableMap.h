#ifndef NONLINFIT_SUM_ABSTRACTMAP_H
#define NONLINFIT_SUM_ABSTRACTMAP_H

#include <Eigen/Core>
#include <vector>
#include <boost/array.hpp>

namespace nonlinfit {
namespace sum {

/** Mapping from parameters on multiple functions to a single parameter sequence.
 *  \tparam VariableCount The number of variables for each input function.
 **/
struct VariableMap {
    typedef std::vector<int> Row;
    std::vector<Row> map;
    int input_var_c;
    int output_var_c;

  public:
    VariableMap(int input_variable_count) : input_var_c(input_variable_count), output_var_c(0) {}
    /** Construct by calling add_function() function_count times. */
    VariableMap(int function_count, std::vector<bool> common );
    /** Add a function that has all bitset-indicated variables in common with
     *  the first function. */
    void add_function( std::vector<bool> common );
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

    bool variables_are_dropped() const;
};

}
}

#endif
