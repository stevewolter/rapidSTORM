#ifndef SIMPARM_MINMAXWATCHER_DECL_HH
#define SIMPARM_MINMAXWATCHER_DECL_HH

#include <boost/type_traits/is_fundamental.hpp>
#include <boost/utility/enable_if.hpp>

namespace simparm {

template <typename Type>
inline typename boost::enable_if< boost::is_fundamental<Type>, bool>::type
exceeds( const Type& a, const Type& b );
template <typename Type>
inline typename boost::enable_if< boost::is_fundamental<Type>, bool>::type
falls_below( const Type& a, const Type& b );

template <typename Bounder, typename Boundee, bool Less>
class BoundWatcher;

};

#endif
