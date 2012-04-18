#ifndef DSTORM_TRAITS_FitResidues_H
#define DSTORM_TRAITS_FitResidues_H

#include "base.h"
#include <boost/units/systems/si/dimensionless.hpp>
#include "no_range.h"

namespace dStorm {
namespace traits {

struct FitResidues;
template <> struct value< FitResidues > :
    public Value< quantity<si::dimensionless, double> > {};

struct FitResidues 
: public value<FitResidues>,
  public NoRange<FitResidues>
{
    static std::string get_ident();
    static std::string get_desc();
    static std::string get_shorthand();
    static const ValueType default_value;
};

}
}

#endif
