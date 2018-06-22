#ifndef NONLINFIT_SUM_ABSTRACTFUNCTION_H
#define NONLINFIT_SUM_ABSTRACTFUNCTION_H

#include <nonlinfit/AbstractFunction.h>
#include <boost/static_assert.hpp>
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
template <
    typename _Function,
    typename _Moveable = _Function,
    typename Policy = UnboundedPolicy >
class AbstractFunction
{
    static const int InputVarC = _Function::Derivatives::VariableCount;
    static const int OutputVariableCountMax = 
        ( Policy::PlaneCountMax == Eigen::Dynamic ) 
            ? Eigen::Dynamic
            : Policy::PlaneCountMax * InputVarC;
  public:
    /** The input function type. */
    typedef _Function argument_type;
    typedef _Moveable moveable_type;
    /** The type of the implemented, resulting function. */
    typedef Evaluation<
        typename _Function::Derivatives::Scalar, 
        Eigen::Dynamic, 
        OutputVariableCountMax> Derivatives;
    typedef typename Derivatives::Vector Position;

  private:
    typedef std::vector< argument_type* > Fitters;
    Fitters fitters;
    std::vector< _Moveable* > movers;
    const VariableMap map;
    const int plane_count;

  public:
    AbstractFunction( const VariableMap& variable_map );
    /** Use the supplied fitter as the base fitter for the given index. */
    void set_fitter( int index, argument_type& input, _Moveable& moveable ) {
        fitters[index] = &input;
        movers[index] = &moveable;
    }
    /** Set all base fitters to the elements in the supplied range. */
    template <typename Iterator>
    void set_fitters( Iterator i, Iterator end ) {
        typename Fitters::iterator j = fitters.begin();
        typename std::vector< _Moveable* >::iterator k = movers.begin();
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
