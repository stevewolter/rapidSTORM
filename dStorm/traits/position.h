#ifndef DSTORM_TRAITS_POSITION_H
#define DSTORM_TRAITS_POSITION_H

#include "../types/samplepos.h"
#include "base.h"
#include "range.h"
#include <boost/units/unit.hpp>
#include <boost/units/make_scaled_unit.hpp>
#include <boost/units/systems/camera/pixel_size.hpp>
#include "../units/nanoresolution.h"
#include "../units/nanolength.h"
#include <boost/units/Eigen/Core>

namespace dStorm {
namespace traits {

template <int Dimension>
struct Position 
: public Value<samplepos::Scalar>,
  public Range<samplepos::Scalar>
{
    typedef quantity< si::nanolength, float > OutputType;

    static std::string get_ident();
    static std::string get_desc();
    static std::string get_shorthand();
    static const samplepos::Scalar default_value;
};

typedef Position<0> PositionX;
typedef Position<1> PositionY;
typedef Position<2> PositionZ;

}
}

#endif
