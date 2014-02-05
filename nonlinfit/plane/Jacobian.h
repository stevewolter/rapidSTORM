#ifndef NONLINFIT_PLANE_JACOBIANCOMPUTER_H
#define NONLINFIT_PLANE_JACOBIANCOMPUTER_H

#include "nonlinfit/plane/Joint.h"
#include "nonlinfit/plane/Disjoint.hpp"
#include <nonlinfit/derive_by.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>

namespace nonlinfit {
namespace plane {

template <typename Lambda, typename Tag>
class Jacobian;

/** Computational optimization of Jacobian matrix for disjoint computation.
 *  This class computes a Jacobian matrix just like nonlinfit::Jacobian,
 *  but cheapens the computation by putting the P2-independent factors into
 *  a statically precomputed matrix (#dx).
 *
 *  The computation of these static parts must be triggered explicitly by
 *  calling the precompute() method. Otherwise, the usage is identical
 *  to nonlinfit::Jacobian. */
template <typename Lambda, typename Num, int ChunkSize, class P1, class P2>
class Jacobian< Lambda, Disjoint<Num, ChunkSize, P1, P2 > >
{
    typedef Disjoint<Num,ChunkSize,P1,P2> Tag;
    typedef typename Tag::template make_derivative_terms<Lambda,P1>::type 
        OuterTerms;
    typedef typename Tag::template make_derivative_terms<Lambda,P2>::type 
        InnerTerms;
    static const int TermCount = boost::mpl::size<OuterTerms>::size::value ;
    static const int VariableCount = 
        boost::mpl::size<typename Lambda::Variables>::type::value;

    typedef nonlinfit::Jacobian<Num, ChunkSize,OuterTerms> OuterJacobian;
    typedef Eigen::Matrix< Num, ChunkSize, VariableCount >
        result_type;
    typedef typename get_evaluator< Lambda, Tag >::type Evaluator;

    typename Tag::template get_derivative_combiner<Lambda>::type combiner;
    OuterJacobian dx;
    result_type result;

  public:
    void precompute( Evaluator& evaluator ) {
        dx.compute(evaluator);
    }
    void compute( Evaluator& evaluator ) {
        /* Compute the Y parts of the derivatives by part. */
        nonlinfit::Jacobian<Num, 1,InnerTerms> dy;
        dy.compute( evaluator );

        /* Sum the contributions from the different derivation summands for
        * each parameter. */
        combiner.row_vector( result, *dx * dy->asDiagonal() );
    }
    const result_type& jacobian() const { return result; }
    const result_type& operator*() const { return result; }
    const result_type* operator->() const { return &result; }
};

/** Forward of nonlinfit::Jacobian.
 *  This class is provided for interface compatibility to the Disjoint 
 *  specialization and functionally equivalent to nonlinfit::Jacobian. */
template <typename Lambda, typename Num, int ChunkSize, class P1, class P2>
class Jacobian< Lambda, Joint<Num,ChunkSize, P1, P2 > >
: public nonlinfit::Jacobian< Num, ChunkSize, typename Lambda::Variables >
{
  public:
    template <typename Evaluator> void precompute( const Evaluator& ) {}
};

}
}

#endif
