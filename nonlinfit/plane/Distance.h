#ifndef NONLINFIT_EVALUATORS_PLANE_GENERIC_H
#define NONLINFIT_EVALUATORS_PLANE_GENERIC_H

#include "nonlinfit/AbstractFunction.h"
#include "nonlinfit/DataChunk.h"
#include "nonlinfit/Evaluation.h"
#include "nonlinfit/plane/fwd.h"
#include "nonlinfit/plane/Term.h"

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
template <typename _Tag, typename _Metric>
class Distance
: public nonlinfit::AbstractFunction<typename _Tag::Number>
{
    typedef _Tag Tag;
    typedef typename _Tag::Number Number;
    typedef typename Tag::Data Data;

    typedef Evaluation< Number > Derivatives;
    typedef typename Derivatives::Vector Position;
    typedef typename Data::DataRow DataRow;
    typedef nonlinfit::DataChunk<Number, Tag::ChunkSize> DataChunk;
    const Data* xs;
    const std::vector<DataChunk>* ys;
    std::vector<Term<Tag>*> terms;
    int variable_count_;

    mutable Eigen::Matrix<Number, Tag::ChunkSize, Eigen::Dynamic> jacobian;

  public:
    Distance( Term<Tag>* term ) : terms(1, term) {
        variable_count_ = 0;
        for (const auto term : terms) {
            variable_count_ += term->variable_count;
        }
        jacobian.resize(Tag::ChunkSize, variable_count_);
    }

    bool evaluate( Derivatives& p );
    void set_data( const Data& xs, const std::vector<DataChunk>& ys ) { this->xs = &xs; this->ys = &ys; }
    int variable_count() const { return variable_count_; }
    void get_position( Position& p ) const OVERRIDE;
    void set_position( const Position& p ) OVERRIDE;
    bool step_is_negligible( const Position& old_position,
                             const Position& new_position ) const OVERRIDE;

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
template <typename Num, int _ChunkSize, typename P1, typename P2>
class Distance< Disjoint<Num,_ChunkSize,P1,P2>, squared_deviations >
: public nonlinfit::AbstractFunction<Num>
{
    typedef Disjoint<Num,_ChunkSize,P1,P2> Tag;
  public:
    // These typedefs are used by nonlinfit::BoundFunction
    typedef Num Number;
    typedef typename Tag::Data Data;

  private:
    typedef Evaluation< Num > Derivatives;
    typedef typename Evaluation< Num >::Vector Position;
    typedef typename Data::DataRow DataRow;
    typedef nonlinfit::DataChunk<Num, _ChunkSize> DataChunk;

    Eigen::Matrix<int, Eigen::Dynamic, 1> reduction;

    /** The x-dependent parts of the jacobian. */
    mutable Eigen::Matrix<Number, Tag::ChunkSize, Eigen::Dynamic> x_jacobian;
    /** The y-dependent parts of the jacobian. */
    mutable Eigen::Matrix<Number, 1, Eigen::Dynamic> y_jacobian_row;
    /** Accumulator for the derivative terms for Evaluation::gradient.
     *  This variable is necessary because a variable might have multiple
     *  terms, making a post-processing step necessary. */
    mutable Eigen::Matrix<Num, Eigen::Dynamic, 1> gradient_accum;
    /** Accumulator for the X parts of the hessian. */
    mutable Eigen::Matrix<Num, Eigen::Dynamic, Eigen::Dynamic> x_hessian;
    /** Accumulator for the Y parts of the hessian. */
    mutable Eigen::Matrix<Num, Eigen::Dynamic, Eigen::Dynamic> y_hessian;

    const Data* xs;
    const std::vector<DataChunk>* ys;
    std::vector<Term<Tag>*> terms;
    int output_variable_count_;

  public:
    Distance(Term<Tag>* term) : terms(1, term) {
        int term_variable_count = 0;
        for (const auto term : terms) {
            term_variable_count += term->term_variable_count;
        }
        x_jacobian.resize(Tag::ChunkSize, term_variable_count);
        y_jacobian_row.resize(1, term_variable_count);
        gradient_accum.resize(term_variable_count);
        x_hessian.resize(term_variable_count, term_variable_count);
        y_hessian.resize(term_variable_count, term_variable_count);
        reduction.resize(term_variable_count);

        int offset = 0;
        output_variable_count_ = 0;
        for (const auto term : terms) {
            auto block = reduction.segment(offset, term->term_variable_count);
            block = term->get_reduction_term();
            block.array() += output_variable_count_;
            offset += term->term_variable_count;
            output_variable_count_ += term->variable_count;
        }
    }

    bool evaluate( Derivatives& p );
    void set_data( const Data& xs, const std::vector<DataChunk>& ys ) { this->xs = &xs; this->ys = &ys; }
    int variable_count() const { return output_variable_count_; }
    void get_position( Position& p ) const;
    void set_position( const Position& p );
    bool step_is_negligible( const Position& old_position,
                             const Position& new_position ) const OVERRIDE;

    typedef void result_type;
    inline void evaluate_chunk( Derivatives&, const DataRow&, const DataChunk& );

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}
}

#endif
