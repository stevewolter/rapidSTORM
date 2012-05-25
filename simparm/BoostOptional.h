#ifndef SIMPARM_ATTRIBUTE_OPTIONAL_HH
#define SIMPARM_ATTRIBUTE_OPTIONAL_HH

namespace boost { template <typename T> class optional; }

namespace simparm {

template <typename Type>
struct add_boost_optional { typedef boost::optional<Type> type; };
template <typename Type>
struct add_boost_optional< boost::optional<Type> > { typedef boost::optional<Type> type; };

}

#endif
