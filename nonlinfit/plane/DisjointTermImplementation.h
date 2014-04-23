#ifndef NONLINFIT_PLANE_DISJOINTTERMIMPLEMENTATION_H
#define NONLINFIT_PLANE_DISJOINTTERMIMPLEMENTATION_H

#include "nonlinfit/plane/DisjointTerm.h"

#include "nonlinfit/plane/Jacobian.h"
#include "nonlinfit/VectorPosition.h"

namespace nonlinfit {
namespace plane {

template <typename Lambda, typename Tag>
class DisjointTermImplementation : public DisjointTerm<Tag> {
    typedef typename Tag::Number Number;
    typedef typename get_evaluator<Lambda, Tag>::type Evaluator;

    typedef typename DisjointTerm<Tag>::ValueVector ValueVector;
    typedef typename DisjointTerm<Tag>::XJacobianBlock XJacobianBlock;
    typedef typename DisjointTerm<Tag>::YJacobianBlock YJacobianBlock;
    typedef typename DisjointTerm<Tag>::PositionBlock PositionBlock;
    typedef typename DisjointTerm<Tag>::ConstPositionBlock ConstPositionBlock;

    typedef typename Tag::template make_derivative_terms<Lambda,typename Tag::OuterParam>::type 
        OuterTerms;
    typedef typename Tag::template make_derivative_terms<Lambda,typename Tag::InnerParam>::type 
        InnerTerms;
    typedef nonlinfit::Jacobian<Number, Tag::ChunkSize,OuterTerms> OuterJacobian;
    static const int TermCount = boost::mpl::size<OuterTerms>::size::value ;

    Evaluator evaluator;
    Jacobian<Lambda, Tag> jacobian_computer;
    VectorPosition<Lambda, Number> mover;

  public:
    DisjointTermImplementation(Lambda& lambda)
        : DisjointTerm<Tag>(TermCount, boost::mpl::size<typename Lambda::Variables>::value),
          evaluator(lambda),
          mover(lambda) {}

    bool prepare_iteration(
            const typename Tag::Data& inputs,
            XJacobianBlock x_jacobian
            ) OVERRIDE {
        if (!evaluator.prepare_iteration(inputs)) {
            return false;
        }

        /* Pre-compute the outer jacobian here so it will be available to compute
        * the residues in the inner loop. */
        OuterJacobian dx;
        dx.compute( evaluator, x_jacobian );

        return true;
    }

    void evaluate_chunk(
            const typename Tag::Data::Input& data,
            ValueVector& values,
            YJacobianBlock jacobian_block) OVERRIDE {
        nonlinfit::Jacobian<Number,1,InnerTerms> dy;
        evaluator.prepare_chunk( data );
        evaluator.add_value( values );
        dy.compute( evaluator, jacobian_block );
    }

    void set_position(ConstPositionBlock position) OVERRIDE {
        typename AbstractFunction<Number>::Position mover_position = position;
        mover.set_position(mover_position);
    }

    void get_position(PositionBlock position) const OVERRIDE {
        typename AbstractFunction<Number>::Position mover_position(position.rows());
        mover.get_position(mover_position);
        position = mover_position;
    }

    Eigen::VectorXi get_reduction_term() const OVERRIDE {
        return MatrixReducer::create_reduction_list<ReducerTag<Tag, Lambda>>();
    }
};

}
}

#endif
