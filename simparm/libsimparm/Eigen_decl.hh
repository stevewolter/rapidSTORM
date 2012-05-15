#ifndef SIMPARM_EIGEN_DECL_HH
#define SIMPARM_EIGEN_DECL_HH

#include <iostream>
#include <Eigen/Core>

namespace simparm {

template <typename Derived>
std::istream& from_config_stream( std::istream& i, Eigen::MatrixBase<Derived>& t );
template <typename Derived>
std::ostream& to_config_stream( std::ostream& o, const Eigen::MatrixBase<Derived>& t );
template <typename Derived>
static const char *typeName( const Eigen::MatrixBase<Derived>& );

template <typename Derived>
bool exceeds( const Eigen::MatrixBase<Derived>& a, const Eigen::MatrixBase<Derived>& b );
template <typename Derived>
bool falls_below( const Eigen::MatrixBase<Derived>& a, const Eigen::MatrixBase<Derived>& b );

template <typename Derived> inline Derived default_value( Eigen::MatrixBase<Derived> );
template <typename Derived> inline Derived default_increment( Eigen::MatrixBase<Derived> );

}

#endif
