#ifndef SIMPARM_DEFAULT_VALUE_HH
#define SIMPARM_DEFAULT_VALUE_HH

#include <boost/type_traits/is_fundamental.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/utility/enable_if.hpp>

namespace simparm {

template <typename NumType>
inline typename boost::enable_if< boost::is_fundamental<NumType>, NumType >::type
default_value( NumType ) { return NumType(0); }

template <typename NumType>
inline typename boost::enable_if< boost::is_fundamental<NumType>, NumType >::type
default_increment( NumType ) { 
    return NumType( (boost::is_integral<NumType>::value) ? 1 : 0 ); 
}

}

#endif
