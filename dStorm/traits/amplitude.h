#ifndef DSTORM_TRAITS_AMPLITUDE_H
#define DSTORM_TRAITS_AMPLITUDE_H

#include "base.h"
#include "range.h"
#include <boost/units/systems/camera/intensity.hpp>
#include <boost/units/quantity.hpp>

namespace dStorm {
namespace traits {

struct Amplitude 
: public Value< quantity< camera::intensity, float > >,
  public Range< quantity< camera::intensity, float > >
{
    static std::string get_ident();
    static std::string get_desc();
    static std::string get_shorthand();
    static const ValueType default_value;
};

}
}

#endif
