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

namespace Eigen {
template <typename Type>
struct NumTraits< boost::optional<Type> > : public NumTraits< Type > {};
}

namespace dStorm {
namespace traits {

template <int Dimension> struct Position;
template <int Dimension> struct value< Position<Dimension> > :
    public Value< samplepos::Scalar > {};

template <int Dimension>
struct Position 
: public Value<samplepos::Scalar>,
  public Range<Position<Dimension> >
{
    typedef quantity< si::nanolength, float > OutputType;
    typedef quantity< nanometer_pixel_size, float > user_resolution_type;

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
