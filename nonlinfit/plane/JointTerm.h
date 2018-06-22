#ifndef NONLINFIT_TERM_H
#define NONLINFIT_TERM_H

#include <Eigen/Core>

namespace nonlinfit {
namespace plane {

template <typename Tag>
class JointTerm {
  private:
    typedef typename Tag::Number Number;
    typedef Eigen::Matrix<Number, Tag::ChunkSize, Eigen::Dynamic> FullJacobian;
    typedef Eigen::Matrix<Number, Eigen::Dynamic, 1> FullPosition;

  protected:
    typedef Eigen::Block<FullJacobian, Tag::ChunkSize> JacobianBlock;
    typedef Eigen::Block<FullPosition, Eigen::Dynamic, 1> PositionBlock;
    typedef Eigen::Block<const FullPosition, Eigen::Dynamic, 1> ConstPositionBlock;
    typedef Eigen::Array<Number, Tag::ChunkSize, 1> ValueVector;

  public:
    const int variable_count;

    JointTerm(int variable_count) : variable_count(variable_count) {}
    virtual ~JointTerm() {}
    virtual bool prepare_iteration(const typename Tag::Data&) = 0;
    virtual void evaluate_chunk(
        const typename Tag::Data::Input& data,
        ValueVector& values,
        JacobianBlock jacobian) = 0;
    virtual void set_position(ConstPositionBlock position) = 0;
    virtual void get_position(PositionBlock position) const = 0;
};

}
}

#endif
