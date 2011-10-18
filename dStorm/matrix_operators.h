#ifndef DSTORM_MATRIX_OPERATORS_H
#define DSTORM_MATRIX_OPERATORS_H

#include <Eigen/Core>

namespace Eigen {

template<typename Scalar>
struct ei_ceil_op EIGEN_EMPTY_STRUCT {
  typedef Scalar result_type;
  EIGEN_STRONG_INLINE const Scalar operator() (const Scalar& a) const
    { return ceil( a ); }
};

template<typename Quantity>
struct ei_functor_traits<ei_ceil_op<Quantity> >
{ enum { Cost = 1, PacketAccess = false }; };

}

namespace std {
template <typename Derived>
EIGEN_STRONG_INLINE const Eigen::CwiseUnaryOp<Eigen::ei_ceil_op<typename Eigen::ei_traits<Derived>::Scalar>, Derived>
ceil( const Eigen::MatrixBase<Derived>&  a)
{
  return a.derived();
};

}

namespace Eigen {

template<typename Scalar>
struct ei_floor_op EIGEN_EMPTY_STRUCT {
  typedef Scalar result_type;
  EIGEN_STRONG_INLINE const Scalar operator() (const Scalar& a) const
    { return floor( a ); }
};

template<typename Quantity>
struct ei_functor_traits<ei_floor_op<Quantity> >
{ enum { Cost = 1, PacketAccess = false }; };

}

namespace std {
template <typename Derived>
EIGEN_STRONG_INLINE const Eigen::CwiseUnaryOp<Eigen::ei_floor_op<typename Eigen::ei_traits<Derived>::Scalar>, Derived>
floor( const Eigen::MatrixBase<Derived>&  a)
{
  return a.derived();
};

}

namespace Eigen {

template<typename Scalar>
struct ei_round_op EIGEN_EMPTY_STRUCT {
  typedef Scalar result_type;
  EIGEN_STRONG_INLINE const Scalar operator() (const Scalar& a) const
    { return round( a ); }
};

template<typename Quantity>
struct ei_functor_traits<ei_round_op<Quantity> >
{ enum { Cost = 1, PacketAccess = false }; };

}

namespace std {
template <typename Derived>
EIGEN_STRONG_INLINE const Eigen::CwiseUnaryOp<Eigen::ei_round_op<typename Eigen::ei_traits<Derived>::Scalar>, Derived>
round( const Eigen::MatrixBase<Derived>&  a)
{
  return a.derived();
};

}

#endif
