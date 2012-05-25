#ifndef SIMPARM_IS_NUMERICAL_H
#define SIMPARM_IS_NUMERICAL_H

#include <boost/optional/optional.hpp>
#include <boost/units/quantity.hpp>
#include <Eigen/Core>

namespace simparm {

inline bool is_numerical( const bool &) { return false; }
template <typename Type>
inline typename boost::enable_if< boost::is_integral<Type>, bool >::type
is_numerical(const Type&) { return true; }
template <typename Type>
inline typename boost::enable_if< boost::is_floating_point<Type>, bool >::type
is_numerical(const Type&) { return true; }
inline bool is_numerical( const std::string& ) { return false; }

template <typename Derived> inline bool is_numerical( const Eigen::MatrixBase<Derived>& );
template <typename Inner> inline bool is_numerical( boost::optional<Inner> );
template <typename Unit, typename Value> inline bool is_numerical( boost::units::quantity<Unit,Value> );

template <typename Inner>
inline bool is_numerical( boost::optional<Inner> ) { return is_numerical(Inner()); }

template <typename Derived>
inline bool is_numerical( const Eigen::MatrixBase<Derived>& ) {
   return is_numerical( typename Derived::Scalar() );
}

template <typename Unit, typename Value>
inline bool is_numerical( boost::units::quantity<Unit,Value> ) { return is_numerical(Value()); }

}

#endif
