#ifndef NONLINFIT_EVALUATORS_PLANE_GENERIC_H
#define NONLINFIT_EVALUATORS_PLANE_GENERIC_H

#include "nonlinfit/AbstractFunction.h"
#include "nonlinfit/DataChunk.h"
#include "nonlinfit/Evaluation.h"
#include "nonlinfit/Evaluator.h"
#include "nonlinfit/plane/fwd.h"
#include "nonlinfit/plane/Jacobian.h"
#include "nonlinfit/VectorPosition.h"

#include "nonlinfit/plane/JointTermImplementation.h"

namespace nonlinfit {
namespace plane {

/** nonlinfit::Function for the distance between a model and planar data.
 *  
 *  This function adapts a nonlinfit::Lambda f and returns the distance
 *  between f's values and a set of input data. The distance is computed
 *  by comparing the value of f with the data's output value and applying
 *  a metric such as plane::squared_deviations or 
 *  plane::inverse_poisson_likelihood.
 *
 *  \tparam _Lambda   A Lambda describing the fitted model
 *  \tparam _Tag      A computation way (Joint or Disjoint)
 *  \tparam _Metric   A metric tag
 **/
template <typename _Lambda, typename _Tag, typename _Metric>
class Distance
: public nonlinfit::AbstractFunction<typename _Tag::Number>
{
    typedef _Tag Tag;
    typedef _Lambda Lambda;
    typedef typename _Tag::Number Number;
    typedef typename Tag::Data Data;

    typedef Evaluation< Number > Derivatives;
    typedef typename Derivatives::Vector Position;
    typedef typename Data::DataRow DataRow;
    typedef nonlinfit::DataChunk<Number, Tag::ChunkSize> DataChunk;
    const Data* xs;
    const std::vector<DataChunk>* ys;
    JointTermImplementation<Lambda, Tag> term;
    std::vector<JointTerm<Tag>*> terms;

    mutable Eigen::Matrix<Number, Tag::ChunkSize, Eigen::Dynamic> jacobian;

  public:
    Distance( Lambda& lambda ) : term(lambda), terms(1, &term) {
        int variable_count = 0;
        for (const auto term : terms) {
            variable_count += term->variable_count;
        }
        jacobian.resize(Tag::ChunkSize, variable_count);
    }

    bool evaluate( Derivatives& p );
    void set_data( const Data& xs, const std::vector<DataChunk>& ys ) { this->xs = &xs; this->ys = &ys; }
    int variable_count() const { return boost::mpl::size<typename Lambda::Variables>::value; }
    void get_position( Position& p ) const OVERRIDE;
    void set_position( const Position& p ) OVERRIDE;

    typedef void result_type;
    inline void evaluate_chunk( Derivatives&, const DataRow&, const DataChunk& );

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

/** Computionally cheaper specialization of Distance. Distance can be computed 
 *  much cheaper for these parameters since the Hessian is a product of 
 *  P1-independent and P2-independent terms. 
 *
 *  \f{eqnarray*}{ 
 *      H_{ij} & = & \sum_x \sum_y \prod_{p \in {P_i,P_j}} 
 *                          \frac{ \partial f(x,y) }{\partial p} \\
 *             & = & \sum_x \sum_y \prod_{p \in {P_i,P_j}}
 *                              \frac{ \partial ( f_x f_y ) }{\partial p} \\
 *             & = & \sum_x \sum_y \prod_{p \in {P_i,P_j}}
 *                              \left( \frac{ \partial f_x }{\partial p} f_y +
 *                                     f_x \frac{ \partial f_y }{\partial p} \right) 
 *  \f}
 *
 *  Since we can always split up parameters such that either f(x) or 
 *  f(y) is independent of any given parameter, only one of the terms
 *  in the above explicit sum will be non-zero. We define a new variable
 *  to make this distinction:
 *
 *  \f{eqnarray*}{
 *      F_{p,x} & = & \left\{ \begin{array}{ll}\frac{\partial f_x}{\partial p} & \textrm{if} f_x \textrm{varies with p} \\
 *                                          f_x & \textrm{else} \end{array} \right. \\
 *      H_{ij} & = & \sum_x \sum_y \prod_{p \in {P_i,P_j}} F_{x,p} F_{y,p} \\
 *      H_{ij} & = & \sum_x \sum_y F_{x,p_i} F_{x,p_j} F_{y,p_i} F_{y,p_j} \\
 *      H_{ij} & = & \left( \sum_x F_{x,p_i} F_{x,p_j} \right) \left( \sum_y F_{y,p_i} F_{y,p_j} \right)
 * \f}
 *
 *  We can compute the parenthesised terms independently, at \f$O(p^2x+p^2y)\f$
 *  complexity instead of \f$O(p^2xy)\f$. This only works for the Hessian in 
 *  squared deviations because the gradient contains residue terms.
 *
 *  Parameters which occur in both the X and the Y factor of the function have
 *  to be duplicated, a duplication that is performed by the Disjoint tag.
 *  While this potentially doubles the number of parameters,
 *  the rise in the number of parameters is usually much smaller.
 **/
template <typename _Lambda, typename Num, int _ChunkSize, typename P1, typename P2>
class Distance< _Lambda,Disjoint<Num,_ChunkSize,P1,P2>, squared_deviations >
: public nonlinfit::AbstractFunction<Num>
{
    typedef Disjoint<Num,_ChunkSize,P1,P2> Tag;
  public:
    // These typedefs are used by nonlinfit::BoundFunction
    typedef _Lambda Lambda;
    typedef Num Number;
    typedef typename Tag::Data Data;

  private:
    typedef Evaluation< Num > Derivatives;
    typedef typename Evaluation< Num >::Vector Position;
    typedef typename Data::DataRow DataRow;
    typedef nonlinfit::DataChunk<Num, _ChunkSize> DataChunk;
    typedef typename Tag::template make_derivative_terms<Lambda,P1>::type 
        OuterTerms;
    typedef typename Tag::template make_derivative_terms<Lambda,P2>::type 
        InnerTerms;
    static const int TermCount = boost::mpl::size<OuterTerms>::size::value ;

    /** Accumulator for the derivative terms for Evaluation::gradient.
     *  This variable is necessary because a variable might have multiple
     *  terms, making a post-processing step necessary. */
    mutable Eigen::Matrix<Num, TermCount, 1> gradient_accum;
    /** Accumulator for the Y parts of the hessian. */
    mutable Eigen::Matrix<Num, TermCount, TermCount> y_hessian;

    typedef nonlinfit::Jacobian<Num, _ChunkSize,OuterTerms> OuterJacobian;

    typedef typename get_evaluator< Lambda, Tag >::type
        Evaluator;
    VectorPosition<Lambda, Num> mover;
    Evaluator evaluator;
    const Data* xs;
    const std::vector<DataChunk>* ys;
    typename Tag::template get_derivative_combiner<Lambda>::type combiner;

  public:
    Distance( Lambda& l ) : mover(l), evaluator(l) {}
    bool evaluate( Derivatives& p );
    void set_data( const Data& xs, const std::vector<DataChunk>& ys ) { this->xs = &xs; this->ys = &ys; }
    int variable_count() const { return boost::mpl::size<typename Lambda::Variables>::value; }
    void get_position( Position& p ) const OVERRIDE { mover.get_position(p); }
    void set_position( const Position& p ) OVERRIDE { mover.set_position(p); }

    typedef void result_type;
    inline void evaluate_chunk( Derivatives&, 
        const OuterJacobian&, const DataRow&, const DataChunk& );

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}
}

#endif
