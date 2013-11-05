#ifndef DSTORM_TRAITS_IMAGENUMBER_H
#define DSTORM_TRAITS_IMAGENUMBER_H

#include "base.h"
#include "range.h"
#include "../units/frame_count.h"
#include <boost/units/systems/camera/frame_rate.hpp>

namespace dStorm {
namespace traits {

struct ImageNumber 
: public Value< quantity<camera::time, int> >,
  public Range< quantity<camera::time, int> >
{
    ImageNumber();
    static std::string get_desc();
    static std::string get_shorthand();

    static const ValueType default_value;
};

}
}

#endif
