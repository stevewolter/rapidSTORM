#ifndef NONLINFIT_SUM_ABSTRACTFUNCTION_H
#define NONLINFIT_SUM_ABSTRACTFUNCTION_H

#include <boost/static_assert.hpp>
#include "nonlinfit/AbstractFunction.h"
#include "nonlinfit/sum/VariableMap.h"

namespace nonlinfit {
namespace sum {

/** A function representing the sum of a dynamic number of equal-type functions.
 *
 *  This class implements a dynamically sized abstract function by summing 
 *  the value of a number of contributing functions. Parameters of different
 *  contributing functions can be declared to be the same number ("bound")
 *  via an VariableMap. The number of contributing functions is chosen at
 *  runtime via the number of functions in the VariableMap.
 *
 *  The contributing functions are not owned by this function.
 */
class AbstractFunction
: public nonlinfit::AbstractFunction<double>
{
  public:
    /** The input function type. */
    typedef nonlinfit::AbstractFunction<double> argument_type;
    /** The type of the implemented, resulting function. */
    typedef Evaluation<double> Derivatives;
    typedef Derivatives::Vector Position;

  private:
    typedef std::vector< argument_type* > Fitters;
    Fitters fitters;
    const VariableMap map;
    const int plane_count;
    mutable typename argument_type::Position position_buffer;
    mutable typename argument_type::Position new_position_buffer;
    mutable typename argument_type::Derivatives evaluation_buffer;
    bool variables_are_dropped;

    void copy_position_into_buffer(const Position&);

  public:
    AbstractFunction( const VariableMap& variable_map );
    /** Use the supplied fitter as the base fitter for the given index. */
    void set_fitter( int index, argument_type& input ) {
        fitters[index] = &input;
    }
    /** Set all base fitters to the elements in the supplied range. */
    template <typename Iterator>
    void set_fitters( Iterator i, Iterator end ) {
        for (auto target = fitters.begin(); i != end; ++i, ++target) {
            *target = &*i;
        }
    }

    int variable_count() const OVERRIDE { return map.output_variable_count(); }
    void get_position( Position& p ) const OVERRIDE;
    void set_position( const Position& p ) OVERRIDE;
    bool evaluate( Derivatives& p ) OVERRIDE;
    bool step_is_negligible( const Position& old_position, const Position& new_position ) const OVERRIDE;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}
}

#endif
