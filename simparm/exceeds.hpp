#ifndef SIMPARM_EXCEEDS_HPP
#define SIMPARM_EXCEEDS_HPP

#include <Eigen/Core>
#include <boost/optional/optional.hpp>
#include <boost/units/quantity.hpp>

namespace simparm {

template <typename Type>
inline typename boost::enable_if< boost::is_fundamental<Type>, bool>::type
exceeds( const Type& a, const Type& b ) {
    return a > b;
}

template <typename Derived>
inline bool exceeds( const Eigen::MatrixBase<Derived>& a, const Eigen::MatrixBase<Derived>& b ) {
    return (a.array() > b.array()).any();
}

template <typename Unit, typename Value>
inline bool exceeds( const boost::units::quantity<Unit,Value>& a, const boost::units::quantity<Unit,Value>& b ) {
    return a > b;
}

template <typename Inner>
inline bool exceeds( const boost::optional<Inner>& a, const boost::optional<Inner>& b ) { 
    return a.is_initialized() && b.is_initialized() && exceeds(*a, *b);
}
template <typename Inner>
inline bool exceeds( const Inner& a, const boost::optional<Inner>& b ) { 
    return b.is_initialized() && exceeds(a, *b);
}
template <typename Inner>
inline bool exceeds( const boost::optional<Inner>& a, const Inner& b ) { 
    return a.is_initialized() && exceeds(*a, b);
}

}

#endif

