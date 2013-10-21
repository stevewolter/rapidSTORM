#ifndef DSTORM_TRAITS_FitResidues_H
#define DSTORM_TRAITS_FitResidues_H

#include "base.h"
#include <boost/units/systems/si/dimensionless.hpp>
#include "no_range.h"

namespace dStorm {
namespace traits {

struct FitResidues 
: public Value< quantity<si::dimensionless, double> >,
  public NoRange< quantity<si::dimensionless, double> >
{
    static std::string get_ident();
    static std::string get_desc();
    static std::string get_shorthand();
    static const ValueType default_value;
};

}
}

#endif
