#ifndef DSTORM_TRAITS_CovarianceMatrix_H
#define DSTORM_TRAITS_CovarianceMatrix_H

#include "base.h"
#include "no_resolution.h"
#include "no_range.h"
#include <boost/units/systems/si/area.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/power10.hpp>

namespace dStorm {
namespace traits {

struct CovarianceMatrix;
template <> struct value< CovarianceMatrix > :
    public Value< quantity< si::area, float >, 2, 2 > {};

struct CovarianceMatrix 
: public value<CovarianceMatrix>,
  public NoResolution<CovarianceMatrix>,
  public NoRange<CovarianceMatrix>
{
    typedef quantity< power_typeof_helper< power10< si::length, -6 >::type, static_rational<2> >::type, float > OutputType;
    static std::string get_ident();
    static std::string get_desc();
    static std::string get_shorthand();
    static const ValueType default_value;
};

}
}

#endif
