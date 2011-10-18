#include "debug.h"
#include "ImageTraits.h"
#include "units/nanolength.h"
#include <boost/units/io.hpp>

namespace dStorm {
namespace input {

GenericImageTraits::GenericImageTraits()
: dim(1) {}

template <int Dimensions>
ImageTraits< Dimensions >::ImageTraits() {}

template ImageTraits<1>::ImageTraits();
template ImageTraits<2>::ImageTraits();
template ImageTraits<3>::ImageTraits();

}
}
