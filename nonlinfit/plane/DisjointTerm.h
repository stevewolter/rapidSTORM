#ifndef NONLINFIT_PLANE_DISJOINT_TERM_H
#define NONLINFIT_PLANE_DISJOINT_TERM_H

#include <Eigen/Core>

namespace nonlinfit {
namespace plane {

template <typename Tag>
class DisjointTerm {
  private:
    typedef typename Tag::Number Number;
    typedef Eigen::Matrix<Number, Tag::ChunkSize, Eigen::Dynamic> FullXJacobian;
    typedef Eigen::Matrix<Number, 1, Eigen::Dynamic> FullYJacobian;
    typedef Eigen::Matrix<Number, Eigen::Dynamic, 1> FullPosition;

  protected:
    typedef Eigen::Block<FullXJacobian, Tag::ChunkSize> XJacobianBlock;
    typedef Eigen::Block<FullYJacobian, 1> YJacobianBlock;
    typedef Eigen::Block<FullPosition, Eigen::Dynamic, 1> PositionBlock;
    typedef Eigen::Block<const FullPosition, Eigen::Dynamic, 1> ConstPositionBlock;
    typedef Eigen::Array<Number, Tag::ChunkSize, 1> ValueVector;

  public:
    const int term_variable_count;
    const int output_variable_count;

    DisjointTerm(int term_variable_count, int output_variable_count)
        : term_variable_count(term_variable_count), output_variable_count(output_variable_count) {}
    virtual ~DisjointTerm() {}
    virtual bool prepare_iteration(
            const typename Tag::Data&,
            XJacobianBlock x_jacobian) = 0;
    virtual void evaluate_chunk(
        const typename Tag::Data::Input& data,
        ValueVector& values,
        YJacobianBlock y_jacobian) = 0;
    virtual void set_position(ConstPositionBlock position) = 0;
    virtual void get_position(PositionBlock position) const = 0;
    virtual Eigen::VectorXi get_reduction_term() const = 0;
};

}
}

#endif
