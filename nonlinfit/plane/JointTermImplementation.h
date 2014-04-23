#ifndef NONLINFIT_PLANE_JOINTTERMIMPLEMENTATION_H
#define NONLINFIT_PLANE_JOINTTERMIMPLEMENTATION_H

#include "nonlinfit/plane/JointTerm.h"

#include "nonlinfit/plane/Jacobian.h"
#include "nonlinfit/VectorPosition.h"

namespace nonlinfit {
namespace plane {

template <typename Lambda, typename Tag>
class JointTermImplementation : public JointTerm<Tag> {
    typedef typename Tag::Number Number;
    typedef typename get_evaluator<Lambda, Tag>::type Evaluator;

    typedef typename JointTerm<Tag>::ValueVector ValueVector;
    typedef typename JointTerm<Tag>::JacobianBlock JacobianBlock;
    typedef typename JointTerm<Tag>::PositionBlock PositionBlock;
    typedef typename JointTerm<Tag>::ConstPositionBlock ConstPositionBlock;

    Evaluator evaluator;
    Jacobian<Lambda, Tag> jacobian_computer;
    VectorPosition<Lambda, Number> mover;

  public:
    JointTermImplementation(Lambda& lambda)
        : JointTerm<Tag>(boost::mpl::size<typename Lambda::Variables>::value),
          evaluator(lambda),
          mover(lambda) {}

    bool prepare_iteration(const typename Tag::Data& inputs) OVERRIDE {
        if (!evaluator.prepare_iteration(inputs)) {
            return false;
        }

        jacobian_computer.precompute( evaluator );
        return true;
    }

    void evaluate_chunk(
            const typename Tag::Data::Input& data,
            ValueVector& values,
            JacobianBlock jacobian_block) OVERRIDE {
        evaluator.prepare_chunk(data);
        evaluator.add_value(values);
        jacobian_computer.compute(evaluator);
        jacobian_block = jacobian_computer.jacobian();
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
};

}
}

#endif
