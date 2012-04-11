#ifndef DSTORM_THREED_INFO_TYPES_H
#define DSTORM_THREED_INFO_TYPES_H

#include "fwd.h"
#include <boost/icl/interval_set.hpp>
#include <boost/icl/type_traits/unit_element.hpp>

namespace boost {
namespace icl {
template <typename Unit, typename Quantity>
struct unit_element< units::quantity<Unit,Quantity> > {
    typedef units::quantity<Unit,Quantity> value_type;
    inline static value_type value() { 
        return value_type::from_value( unit_element<Quantity>::value() );
    }
};

}
}

namespace dStorm {
namespace threed_info {

using boost::units::quantity;
namespace si = boost::units::si;

typedef quantity<si::length,float> Sigma;
typedef quantity<si::length,float> ZPosition;
typedef double SigmaDerivative;
typedef boost::icl::interval_set<ZPosition> ZRange;
typedef boost::icl::continuous_interval<ZPosition> ZInterval;

}
}
#endif
