#ifndef DSTORM_TRAITS_CovarianceMatrix_H
#define DSTORM_TRAITS_CovarianceMatrix_H

#include "base.h"
#include "no_range.h"
#include <boost/units/systems/si/area.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/power10.hpp>

namespace dStorm {
namespace traits {

struct PSFWidth;
template <> struct value< PSFWidth > :
    public Value< quantity< si::length, float >, 2, 1 > {};

struct PSFWidth 
: public value<PSFWidth>,
  public NoRange<PSFWidth>
{
    typedef quantity< power10< si::length, -9 >::type, float > OutputType;
    static std::string get_ident();
    static std::string get_desc();
    static std::string get_shorthand();
    static const ValueType default_value;
};

}
}

#endif
