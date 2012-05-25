#ifndef SIMPARM_TYPENAME_H
#define SIMPARM_TYPENAME_H

#include <boost/optional/optional.hpp>
#include <boost/units/quantity.hpp>
#include <Eigen/Core>

namespace simparm {

inline const char *typeName( const bool &) { return "Bool"; }
template <typename Type>
inline typename boost::enable_if< boost::is_integral<Type>, const char * >::type
typeName(const Type&) { return "Integer"; }
template <typename Type>
inline typename boost::enable_if< boost::is_floating_point<Type>, const char * >::type
typeName(const Type&) { return "Float"; }
inline const char *typeName( const std::string& ) { return "String"; }

template <typename Derived> inline const char *typeName( const Eigen::MatrixBase<Derived>& );
template <typename Inner> inline const char *typeName( boost::optional<Inner> );
template <typename Unit, typename Value> inline const char *typeName( boost::units::quantity<Unit,Value> );

template <typename Inner>
inline const char *typeName( boost::optional<Inner> ) { return typeName(Inner()); }

template <typename Derived>
inline const char *typeName( const Eigen::MatrixBase<Derived>& ) {
   return typeName( typename Derived::Scalar() );
}

template <typename Unit, typename Value>
inline const char *typeName( boost::units::quantity<Unit,Value> ) { return typeName(Value()); }

}

#endif
