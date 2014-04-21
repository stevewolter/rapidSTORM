#ifndef NONLINFIT_SUM_ABSTRACTFUNCTION_H
#define NONLINFIT_SUM_ABSTRACTFUNCTION_H

#include <boost/static_assert.hpp>
#include "nonlinfit/AbstractFunction.h"
#include "nonlinfit/AbstractMoveable.h"
#include "nonlinfit/sum/AbstractMap.h"

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
 *  via an AbstractMap. The number of contributing functions is chosen at
 *  runtime via the number of functions in the AbstractMap.
 *
 *  The contributing functions are not owned by this function.
 */
template <
    typename Number,
    typename Policy = UnboundedPolicy >
class AbstractFunction
: public nonlinfit::AbstractFunction<Number>,
  public nonlinfit::AbstractMoveable<Number>
{
    typedef Evaluation<Number> Derivatives;
    typedef typename Derivatives::Vector Position;

    std::vector< nonlinfit::AbstractFunction<Number>* > fitters;
    std::vector< AbstractMoveable<Number>* > movers;
    const AbstractMap map;
    const int plane_count;

  public:
    AbstractFunction( const AbstractMap& variable_map );
    /** Use the supplied fitter as the base fitter for the given index. */
    void set_fitter( int index, nonlinfit::AbstractFunction<Number>* input, AbstractMoveable<Number>* moveable ) {
        fitters[index] = input;
        movers[index] = moveable;
    }
    /** Set all base fitters to the elements in the supplied range. */
    template <typename Iterator>
    void set_fitters( Iterator i, Iterator end ) {
        auto j = fitters.begin();
        auto k = movers.begin();
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
