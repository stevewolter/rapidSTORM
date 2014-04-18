#ifndef DSTORM_UNITS_ICL_H
#define DSTORM_UNITS_ICL_H

#include <boost/icl/type_traits/unit_element.hpp>
#include <boost/icl/type_traits/is_discrete.hpp>

namespace boost {
namespace icl {

template <typename Unit, typename Quantity>
struct unit_element< units::quantity<Unit,Quantity> > {
    typedef units::quantity<Unit,Quantity> value_type;
    inline static value_type value() { 
        return value_type::from_value( unit_element<Quantity>::value() );
    }
};

template <typename Unit, typename Quantity>
struct is_discrete< units::quantity<Unit,Quantity> > 
: public is_discrete<Quantity> {};

}
}

#endif
