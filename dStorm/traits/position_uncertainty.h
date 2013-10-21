#ifndef DSTORM_TRAITS_POSITION_UNCERTAINTY_H
#define DSTORM_TRAITS_POSITION_UNCERTAINTY_H

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
struct PositionUncertainty;

template <int Dimension>
struct value< PositionUncertainty<Dimension> > :
    public Value< quantity<si::length, float> > {};

template <int Dimension>
struct PositionUncertainty
: public value<PositionUncertainty<Dimension> >,
  public NoRange<PositionUncertainty<Dimension> >
{
    typedef quantity< si::nanolength, float > OutputType;
    typedef quantity< nanometer_pixel_size, float > user_resolution_type;

    static std::string get_ident();
    static std::string get_desc();
    static std::string get_shorthand();
    static const quantity<si::length,float> default_value;
};

typedef PositionUncertainty<0> PositionUncertaintyX;
typedef PositionUncertainty<1> PositionUncertaintyY;
typedef PositionUncertainty<2> PositionUncertaintyZ;

}
}

#endif
