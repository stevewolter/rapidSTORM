#ifndef DSTORM_UNIT_MATRIX_OPERATORS_H
#define DSTORM_UNIT_MATRIX_OPERATORS_H

#include <Eigen/Core>
#include <boost/units/quantity.hpp>
#include "units_Eigen_traits.h"

namespace boost {
namespace units {

#if 0
template<typename Unit, typename X, typename Y> 
typename multiply_typeof_helper< quantity< Unit, X >, Y >::type
operator*(const quantity< Unit, X > & lhs, const Y & rhs)
{
    return multiply_typeof_helper< quantity< Unit, X >, Y >::type
        ::from_value( lhs.value() * rhs );
}

template<typename Unit, typename X, typename Y> 
typename multiply_typeof_helper< quantity< Unit, X >, Y >::type
operator*(const Y& lhs, const quantity< Unit, X > & rhs)
{
    return multiply_typeof_helper< quantity< Unit, X >, Y >::type
        ::from_value( lhs * rhs.value() );
}

template<typename Unit, typename X, typename Y> 
typename divide_typeof_helper< quantity< Unit, X >, Y >::type
operator/(const quantity< Unit, X > & lhs, const Y & rhs)
{
    return divide_typeof_helper< quantity< Unit, X >, Y >::type
        ::from_value( lhs.value() / rhs );
}

template<typename Unit, typename X, typename Y> 
typename divide_typeof_helper< quantity< Unit, X >, Y >::type
operator/(const Y& lhs, const quantity< Unit, X > & rhs)
{
    return divide_typeof_helper< quantity< Unit, X >, Y >::type
        ::from_value( lhs / rhs.value() );
}
#endif

}
}

namespace Eigen {

template<typename Quantity>
struct ei_unitless_value_op EIGEN_EMPTY_STRUCT {
  typedef typename Quantity::value_type result_type;
  EIGEN_STRONG_INLINE const typename Quantity::value_type operator() (const Quantity& a) const
    { return a.value(); }
};

template<typename Quantity>
struct ei_functor_traits<ei_unitless_value_op<Quantity> >
{ enum { Cost = 0, PacketAccess = false }; };

template<typename Unit, typename Scalar>
struct ei_from_value_op EIGEN_EMPTY_STRUCT {
  typedef boost::units::quantity<Unit, Scalar> Quantity;
  typedef typename Quantity::this_type result_type;
  EIGEN_STRONG_INLINE const Quantity operator() (const Scalar& a) const
    { return Quantity::from_value( a ); }
};

template<typename Quantity, typename Unit>
struct ei_functor_traits<ei_from_value_op<Quantity, Unit> >
{ enum { Cost = 0, PacketAccess = false }; };

template<typename Quantity, typename NewType>
struct ei_value_cast_op EIGEN_EMPTY_STRUCT {
  typedef typename boost::units::quantity<typename Quantity::unit_type,NewType> result_type;
  EIGEN_STRONG_INLINE const result_type operator() (const Quantity& a) const
    { return result_type( a ); }
};

template<typename Quantity, typename NewType>
struct ei_functor_traits<ei_value_cast_op<Quantity,NewType> >
{ enum { Cost = 0, PacketAccess = false }; };

template <typename Unit>
struct quantity_matrix {

template<typename Derived>
static
EIGEN_STRONG_INLINE const CwiseUnaryOp<ei_from_value_op<Unit, typename ei_traits<Derived>::Scalar>, Derived>
from_value( const MatrixBase<Derived>&  a)
{
  return a.derived();
}

};

template <typename ValueType>
struct value_type {

template<typename Derived>
static
EIGEN_STRONG_INLINE const CwiseUnaryOp<ei_value_cast_op<typename ei_traits<Derived>::Scalar, ValueType>, Derived>
cast( const MatrixBase<Derived>&  a)
{
  return a.derived();
};

};

template<typename Derived>
EIGEN_STRONG_INLINE const CwiseUnaryOp<ei_unitless_value_op<typename ei_traits<Derived>::Scalar>, Derived>
unitless_value( const MatrixBase<Derived>&  a)
{
  return a.derived();
};

namespace op_helper {
template <typename Type>
struct value {
    static const Type& get( const Type& a ) { return a; }
};
template <typename Unit, typename Type>
struct value< boost::units::quantity<Unit,Type> > {
    static const Type& get( const boost::units::quantity<Unit,Type>& a ) { return a.value(); }
};
}

template<typename Unit, typename Scalar, int Rows, int Cols, int Opts, int MaxR, int MaxC, typename Factor>
Eigen::Matrix<
typename boost::units::multiply_typeof_helper< boost::units::quantity<Unit,Scalar>, Factor >::type,
    Rows, Cols, Opts, MaxR, MaxC>
operator*(const Factor& lhs, const Eigen::Matrix<boost::units::quantity<Unit, Scalar>, Rows, Cols, Opts, MaxR, MaxC>& rhs)
{
    return Eigen::quantity_matrix<
        typename boost::units::multiply_typeof_helper< boost::units::quantity<Unit,Scalar>, Factor >::type::unit_type>::from_value
        ( op_helper::value<Factor>::get(lhs) * unitless_value(rhs) );
}

template<typename Unit, typename Scalar, int Rows, int Cols, int Opts, int MaxR, int MaxC, typename Factor>
Eigen::Matrix<
typename boost::units::multiply_typeof_helper< boost::units::quantity<Unit,Scalar>, Factor >::type,
    Rows, Cols, Opts, MaxR, MaxC>
operator*(const Eigen::Matrix<boost::units::quantity<Unit, Scalar>, Rows, Cols, Opts, MaxR, MaxC>& a, const Factor& b)
{
    return Eigen::quantity_matrix<typename boost::units::multiply_typeof_helper< boost::units::quantity<Unit,Scalar>, Factor >::type::unit_type>::from_value
        ( unitless_value(a) * op_helper::value<Factor>::get(b) );
}

template<typename Unit, typename Scalar, int Rows, int Cols, int Opts, int MaxR, int MaxC, typename Factor>
Eigen::Matrix<
typename boost::units::divide_typeof_helper< boost::units::quantity<Unit,Scalar>, Factor >::type,
    Rows, Cols, Opts, MaxR, MaxC>
operator/(const Eigen::Matrix<boost::units::quantity<Unit, Scalar>, Rows, Cols, Opts, MaxR, MaxC>& a, const Factor& b)
{
    return Eigen::quantity_matrix<typename boost::units::multiply_typeof_helper< boost::units::quantity<Unit,Scalar>, Factor >::type::unit_type>::from_value
        ( unitless_value(a) / op_helper::value<Factor>::get(b) );
}

}

#endif
