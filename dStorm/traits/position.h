#ifndef DSTORM_TRAITS_POSITION_H
#define DSTORM_TRAITS_POSITION_H

#include "../types/samplepos.h"
#include "base.h"
#include "resolution.h"
#include "range.h"
#include <boost/units/unit.hpp>
#include <boost/units/make_scaled_unit.hpp>
#include <boost/units/systems/camera/resolution.hpp>
#include <boost/units/systems/camera/pixel_size.hpp>
#include "../units/nanoresolution.h"
#include "../units/nanolength.h"
#include "../units_Eigen_traits.h"

namespace Eigen {
template <typename Type>
struct NumTraits< boost::optional<Type> > : public NumTraits< Type > {};
}

namespace dStorm {
namespace traits {

struct Position;
template <> struct value< Position > :
    public derived_value< samplepos > {};

struct Position 
: public value<Position>,
  public Resolution< Position, quantity< camera::resolution, float > >,
  public Range<Position>
{
    typedef quantity< si::nanolength, float > OutputType;
    typedef quantity< nanometer_pixel_size, float > user_resolution_type;

    quantity< camera::resolution, float > from_user_resolution_unit( const user_resolution_type& u ) 
        { return 1.0f / quantity< camera::pixel_size, float >(u / (1.0f * si::nanometer) * (1e-9f * si::meter) ); }
    user_resolution_type to_user_resolution_unit( const quantity< camera::resolution, float >& u ) 
        { return (1.0f / u) * (1.0f * si::nanometer) / (1e-9f * si::meter); }

    static std::string get_ident();
    static std::string get_desc();
    static std::string get_shorthand();
    static const ValueType default_value;
};

}
}

#endif
