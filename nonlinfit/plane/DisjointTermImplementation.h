#ifndef NONLINFIT_PLANE_DISJOINTTERMIMPLEMENTATION_H
#define NONLINFIT_PLANE_DISJOINTTERMIMPLEMENTATION_H

#include "nonlinfit/plane/Term.h"

#include "nonlinfit/derive_by.hpp"
#include "nonlinfit/get_variable.hpp"
#include "nonlinfit/parameter_is_negligible.hpp"
#include "nonlinfit/plane/Disjoint.hpp"
#include "nonlinfit/plane/DisjointData.h"
#include "nonlinfit/plane/sum_matrix_rows.h"
#include "nonlinfit/set_variable.hpp"

namespace nonlinfit {
namespace plane {

template <typename Lambda, typename Tag>
class DisjointTermImplementation : public Term<Tag> {
    typedef typename Tag::Number Number;
    typedef typename get_evaluator<Lambda, Tag>::type Evaluator;

    typedef typename Term<Tag>::ValueVector ValueVector;
    typedef typename Term<Tag>::JacobianBlock JacobianBlock;
    typedef typename Term<Tag>::JacobianRowBlock JacobianRowBlock;
    typedef typename Term<Tag>::PositionBlock PositionBlock;
    typedef typename Term<Tag>::ConstPositionBlock ConstPositionBlock;

    typedef typename Tag::template make_derivative_terms<Lambda,typename Tag::OuterParam>::type 
        OuterTerms;
    typedef typename Tag::template make_derivative_terms<Lambda,typename Tag::InnerParam>::type 
        InnerTerms;
    static const int TermCount = boost::mpl::size<OuterTerms>::size::value ;

    Lambda& lambda;
    Evaluator evaluator;
    Eigen::Matrix<int, TermCount, 1> reductions;
    Eigen::Matrix<Number, Tag::ChunkSize, TermCount> x_parts;

  public:
    DisjointTermImplementation(Lambda& lambda)
        : Term<Tag>(TermCount, boost::mpl::size<typename Lambda::Variables>::value),
          lambda(lambda),
          evaluator(lambda),
          reductions(MatrixReducer::create_reduction_list<ReducerTag<Tag, Lambda>>()) {}

    bool prepare_iteration(const typename Tag::Data& inputs) {
        if (!evaluator.prepare_iteration(inputs)) {
            return false;
        }

        Jacobian<OuterTerms>::compute(evaluator, x_parts);
        return true;
    }

    bool prepare_disjoint_iteration(
            const typename Tag::Data& inputs,
            JacobianBlock x_jacobian
            ) OVERRIDE {
        if (!evaluator.prepare_iteration(inputs)) {
            return false;
        }

        /* Pre-compute the outer jacobian here so it will be available to compute
        * the residues in the inner loop. */
        Jacobian<OuterTerms>::compute(evaluator, x_jacobian );

        return true;
    }

    void evaluate_chunk(
        const typename Tag::Data::Input& data,
        ValueVector& values,
        JacobianBlock jacobian) {
        evaluator.prepare_chunk(data);
        evaluator.add_value(values);

        /* Compute the Y parts of the derivatives by part. */
        Eigen::Matrix<Number, 1, TermCount> y_parts;
        Jacobian<InnerTerms>::compute(evaluator, y_parts );

        /* Sum the contributions from the different derivation summands for
        * each parameter. */
        sum_cols( jacobian, x_parts * y_parts.asDiagonal(), reductions );
    }

    void evaluate_disjoint_chunk(
            const typename Tag::Data::Input& data,
            ValueVector& values,
            JacobianRowBlock jacobian_block) OVERRIDE {
        evaluator.prepare_chunk( data );
        evaluator.add_value( values );
        Jacobian<InnerTerms>::compute( evaluator, jacobian_block );
    }

    void set_position(ConstPositionBlock position) OVERRIDE {
	set_variable::read_vector(position, lambda);
    }

    void get_position(PositionBlock position) const OVERRIDE {
	get_variable::fill_vector(lambda, position);
    }

    Eigen::VectorXi get_reduction_term() const OVERRIDE {
        return reductions;
    }

    bool step_is_negligible(
            ConstPositionBlock from,
            ConstPositionBlock to) const OVERRIDE {
        return parameter_is_negligible().all(lambda, from, to);
    }
};

}
}

#endif
