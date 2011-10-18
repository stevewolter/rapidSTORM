#ifndef DSTORM_TRAITS_IMAGENUMBER_H
#define DSTORM_TRAITS_IMAGENUMBER_H

#include "base.h"
#include "resolution.h"
#include "range.h"
#include "../units/frame_count.h"
#include <boost/units/systems/camera/frame_rate.hpp>

namespace dStorm {
namespace traits {

struct ImageNumber;
template <> struct value< ImageNumber > :
    public Value< quantity< camera::time, int > > {};

struct ImageNumber 
: public value<ImageNumber>,
  public Resolution< ImageNumber, quantity< camera::frame_rate, float > >,
  public Range< ImageNumber >
{
    ImageNumber();
    static std::string get_ident();
    static std::string get_desc();
    static std::string get_shorthand();

    static const ValueType default_value;
};

}
}

#endif
