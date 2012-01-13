#ifndef NONLINFIT_DERIVATION_SUMMAND_H
#define NONLINFIT_DERIVATION_SUMMAND_H

#include <boost/mpl/int.hpp>
#include <iostream>

namespace nonlinfit {

/** A tag representing a single summand of a disjointly computed parameter's
 *  value.
 *
 *  When computing the values of a multidimensional function disjointly,
 *  each derivative is given by its summands \f$S\f$, the dimensions
 *  \f$D\f$ and its derivative for this tag \f$\delta(S,X)\f$ as:
 *
 *  \f$ \sum_{s \in S} \prod{d \in D} \sum{x \in X(d)} \delta(s,x) \f$
 *
 * In less mathematical terms, the derivatives for each dimension and
 * each summand are accumulated separately, then the accumulations for
 * all dimensions are multiplied with each other, and then the different
 * summands for the parameter are added together.
 *
 *  \tparam _Summand The index of this term from 0 to 
 *                   nonlinfit::derivation_summand_count::type::value
 *  \tparam _Parameter A parameter tag for the to-be-determined derivative
 *  \tparam _Dimension  The spatial dimension that is evaluated. 0 is X, 1 is Y. */
template <class _Summand, typename _Parameter, class _Dimension>
struct DerivationSummand {
   typedef _Summand Summand;
   typedef _Parameter Parameter;
   typedef _Dimension Dimension;
};

template <class _Summand, typename _Parameter, class _Dimension>
inline std::ostream& operator<<(std::ostream& o, DerivationSummand<_Summand,_Parameter,_Dimension>)  
    { return (o << "devsum " << _Summand::value << " in dim " << _Dimension::value << " for " << _Parameter()); }

/** Free metafunction to get the base parameter type of a constructed 
 *  parameter. */
template <typename Term>
struct get_parameter { typedef typename Term::Parameter type; };
}

#endif
