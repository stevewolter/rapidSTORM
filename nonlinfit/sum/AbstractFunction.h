#ifndef NONLINFIT_SUM_ABSTRACTFUNCTION_H
#define NONLINFIT_SUM_ABSTRACTFUNCTION_H

#include <boost/static_assert.hpp>
#include "nonlinfit/AbstractFunction.h"
#include "nonlinfit/AbstractMoveable.h"
#include "nonlinfit/sum/VariableMap.h"

namespace nonlinfit {
namespace sum {

class VariableDropPolicy {
public:
    static const bool VariablesAreDropped = true;
    static const int PlaneCount = Eigen::Dynamic, PlaneCountMax = Eigen::Dynamic;
};

template <int MaxPlaneCount>
class BoundedPolicy {
public:
    static const bool VariablesAreDropped = false;
    static const int PlaneCount = Eigen::Dynamic, PlaneCountMax = MaxPlaneCount;
};

class UnboundedPolicy {
public:
    static const bool VariablesAreDropped = false;
    static const int PlaneCount = Eigen::Dynamic, PlaneCountMax = Eigen::Dynamic;
};

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
template <typename Number, typename Policy = UnboundedPolicy >
class AbstractFunction
: public nonlinfit::AbstractFunction<Number>,
  public AbstractMoveable<Number>
{
  public:
    /** The input function type. */
    typedef nonlinfit::AbstractFunction<Number> argument_type;
    typedef nonlinfit::AbstractMoveable<Number> moveable_type;
    /** The type of the implemented, resulting function. */
    typedef Evaluation<Number> Derivatives;
    typedef typename Derivatives::Vector Position;

  private:
    typedef std::vector< argument_type* > Fitters;
    Fitters fitters;
    std::vector< moveable_type* > movers;
    const VariableMap map;
    const int plane_count;
    mutable typename moveable_type::Position position_buffer;
    mutable typename argument_type::Derivatives evaluation_buffer;

  public:
    AbstractFunction( const VariableMap& variable_map );
    /** Use the supplied fitter as the base fitter for the given index. */
    void set_fitter( int index, argument_type& input, moveable_type& moveable ) {
        fitters[index] = &input;
        movers[index] = &moveable;
    }
    /** Set all base fitters to the elements in the supplied range. */
    template <typename Iterator>
    void set_fitters( Iterator i, Iterator end ) {
        typename Fitters::iterator j = fitters.begin();
        typename std::vector< moveable_type* >::iterator k = movers.begin();
        for ( ; i != end; ++i, ++j, ++k ) { *j = &*i; *k = &*i; }
    }

    int variable_count() const { return map.output_variable_count(); }
    void get_position( Position& p ) const;
    void set_position( const Position& p );
    bool evaluate( Derivatives& p );

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}
}

#endif
