#ifndef DSTORM_TRAITS_CovarianceMatrix_H
#define DSTORM_TRAITS_CovarianceMatrix_H

#include "base.h"
#include "no_range.h"
#include <boost/units/systems/si/area.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/power10.hpp>

namespace dStorm {
namespace traits {

template <int Dimension>
struct PSFWidth 
: public Value< quantity<si::length, float> >,
  public NoRange< quantity<si::length, float> >
{
    typedef quantity< power10< si::length, -9 >::type, float > OutputType;
    static std::string get_desc();
    static std::string get_shorthand();
    static const quantity< si::length, float > default_value;
};

typedef PSFWidth<0> PSFWidthX;
typedef PSFWidth<1> PSFWidthY;

}
}

#endif
