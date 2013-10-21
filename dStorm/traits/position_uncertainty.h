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

struct PositionUncertainty;
template <> struct value< PositionUncertainty > :
    public derived_value< samplepos > {};

struct PositionUncertainty
: public value<PositionUncertainty>,
  public NoRange<PositionUncertainty>
{
    typedef quantity< si::nanolength, float > OutputType;
    typedef quantity< nanometer_pixel_size, float > user_resolution_type;

    static std::string get_ident();
    static std::string get_desc();
    static std::string get_shorthand();
    static const ValueType default_value;
};

}
}

#endif
