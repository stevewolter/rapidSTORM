#ifndef DSTORM_TRAITS_LOCAL_BACKGROUND_H
#define DSTORM_TRAITS_LOCAL_BACKGROUND_H

#include "base.h"
#include "range.h"
#include <boost/units/systems/camera/intensity.hpp>
#include <boost/units/quantity.hpp>

namespace dStorm {
namespace traits {

struct LocalBackground;
template <> struct value< LocalBackground > :
    public Value< quantity< camera::intensity, float > > {};

struct LocalBackground 
: public value<LocalBackground>,
  public Range<LocalBackground>
{
    static std::string get_ident();
    static std::string get_desc();
    static std::string get_shorthand();
    static const ValueType default_value;
};

}
}

#endif
