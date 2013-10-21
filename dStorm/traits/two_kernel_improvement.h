#ifndef DSTORM_TRAITS_TwoKernelImprovement_H
#define DSTORM_TRAITS_TwoKernelImprovement_H

#include "base.h"
#include <boost/units/systems/si/dimensionless.hpp>
#include "no_range.h"

namespace dStorm {
namespace traits {

struct TwoKernelImprovement 
: public Value< quantity<si::dimensionless, float> >,
  public NoRange< quantity<si::dimensionless, float> >
{
    static std::string get_ident();
    static std::string get_desc();
    static std::string get_shorthand();
    static const ValueType default_value;
};

}
}

#endif
