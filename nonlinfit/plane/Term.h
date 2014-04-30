#ifndef NONLINFIT_TERM_H
#define NONLINFIT_TERM_H

#include <Eigen/Core>

namespace nonlinfit {
namespace plane {

template <typename Tag>
class Term {
  private:
    typedef typename Tag::Number Number;
    typedef Eigen::Matrix<Number, Tag::ChunkSize, Eigen::Dynamic> FullJacobian;
    typedef Eigen::Matrix<Number, Eigen::Dynamic, 1> FullPosition;
    typedef Eigen::Matrix<Number, 1, Eigen::Dynamic> FullJacobianRow;

  protected:
    typedef typename Eigen::Ref<FullJacobian, (Tag::ChunkSize % 2 == 0) ? Eigen::Aligned : Eigen::Unaligned> JacobianBlock;
    typedef typename Eigen::Ref<FullJacobianRow> JacobianRowBlock;
    typedef typename FullPosition::SegmentReturnType PositionBlock;
    typedef Eigen::Block<const FullPosition, Eigen::Dynamic, 1> ConstPositionBlock;
    typedef Eigen::Array<Number, Tag::ChunkSize, 1> ValueVector;

  public:
    const int term_variable_count;
    const int variable_count;

    Term(int term_variable_count, int variable_count)
        : term_variable_count(term_variable_count), variable_count(variable_count) {}
    virtual ~Term() {}
    virtual bool prepare_iteration(const typename Tag::Data&) = 0;
    virtual bool prepare_disjoint_iteration(
            const typename Tag::Data&,
            JacobianBlock x_jacobian) = 0;
    virtual void evaluate_chunk(
        const typename Tag::Data::Input& data,
        ValueVector& values,
        JacobianBlock jacobian) = 0;
    virtual void evaluate_disjoint_chunk(
        const typename Tag::Data::Input& data,
        ValueVector& values,
        JacobianRowBlock y_jacobian) = 0;
    virtual void set_position(ConstPositionBlock position) = 0;
    virtual void get_position(PositionBlock position) const = 0;
    virtual Eigen::VectorXi get_reduction_term() const = 0;
    virtual bool step_is_negligible(
            ConstPositionBlock from,
            ConstPositionBlock to) const = 0;
};

}
}

#endif
