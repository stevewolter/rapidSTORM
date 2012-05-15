#ifndef SIMPARM_UNIT_DECLARATOR_HH
#define SIMPARM_UNIT_DECLARATOR_HH

#include <boost/type_traits/is_fundamental.hpp>
#include <boost/utility/enable_if.hpp>
#include "Node.hh"

namespace simparm {

template <typename Type, class Enable = void>
struct UnitDeclarator;

template <typename Type>
struct UnitDeclarator<Type, typename boost::enable_if< boost::is_fundamental<Type> >::type >
{
   void registerNamedEntries( simparm::Node& ) {}
};

}

#endif
