#ifndef NONLINFIT_PLANE_DISJOINT_H
#define NONLINFIT_PLANE_DISJOINT_H

#include "nonlinfit/plane/fwd.h"
#include <nonlinfit/Xs.h>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>

namespace nonlinfit {
namespace plane {

/** ComputationWay for disjoint computing on a plane.
 *  Disjoint computation is the separate computation of the X and the Y
 *  parts of a function, which are then multiplied together. The simple
 *  bilinear function (\f$f(x,y) = x y\f$) is a typical example
 *  for such a function. If this function is to be evaluated for a 
 *  rectangular lattice in a plane (i.e. for all x,y combinations from
 *  two coordinate lists X and Y), the value and partial derivatives can
 *  be computed much more cheaply by the multiplicative associativity.
 *
 *  This class treats its two different parameters differently. The number of
 *  distinct values for the first parameter is fixed by _ChunkSize and given
 *  as a vector in the data. The second parameter describes a variable-sized
 *  component, accessible by the data's iterator.
 *
 *  nonlinfit::Evaluator instances for Disjoint will get 
 *  nonlinfit::DerivationSummand instances as derivative tags. The dimension
 *  type of the DerivationSummand will be set to one of OuterParam and 
 *  InnerParam, and the computed derivative must depend only on the given
 *  dimension. Given a parameter p, the set of dimensions D, the set
 *  of the parameter's summands S and the Evaluator instance e, the expression 
 *  \f$ \sum_{s \in S(p)} \prod{d \in D(p)} e(s,p,d) \f$ must give the full
 *  partial derivative with respect to p.
 *
 *  Example: The function \f$f(x,y) = (x+a) (y+a)\f$ has two summands
 *  for the derivative of $a$. In the X dimension, the summands are
 *  \f${ 1, x+a }\f$ for X and \f${ y+a, 1 }\f$ for Y, giving the
 *  total derivative \f$ y+a + x+a \f$.
 *  
 *  \tparam _ChunkSize Number of different values for the InnerParam.
 *  \tparam OuterParam The parameter describing the fixed-size component.
 *  \tparam InnerParam The parameter describing the variable-size component.
 *  \sa Joint
 **/
template <typename Num, int _ChunkSize, typename OuterParam_, typename InnerParam_>
struct Disjoint {
    typedef Num Number;
    static const int ChunkSize = _ChunkSize;
    typedef DisjointData<Num,ChunkSize> Data;
    typedef OuterParam_ OuterParam;
    typedef InnerParam_ InnerParam;

    template <typename Function, typename Parameter>
    struct make_derivative_terms;

    template <typename Function>
    struct get_derivative_combiner;
};

/** Metafunction returning an instance of Disjoint with Xs as the
 *  parameters. Xs<0> is the fixed-size parameter. */
template <typename Num, int _ChunkSize>
struct xs_disjoint {
    typedef Disjoint<Num,_ChunkSize, Xs<0>, Xs<1> > type;
};

}
}

#endif
