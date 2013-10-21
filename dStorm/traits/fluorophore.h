
#ifndef DSTORM_TRAITS_FLUOROPHORE
#define DSTORM_TRAITS_FLUOROPHORE

#include "../types/fluorophore.h"
#include "base.h"
#include "range.h"

namespace dStorm {
namespace traits {

struct Fluorophore 
: public Value< dStorm::Fluorophore >,
  public Range< dStorm::Fluorophore >
{
    typedef dStorm::Fluorophore OutputType;

    static std::string get_ident();
    static std::string get_desc();
    static std::string get_shorthand();
    static const ValueType default_value;
};

}
}

#endif

