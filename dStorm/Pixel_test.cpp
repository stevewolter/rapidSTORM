#ifdef NDEBUG
#undef NDEBUG
#endif
#include "Pixel.h"
#include <iostream>
#include <cassert>
#include "dejagnu.h"

using namespace dStorm;

void pixel_unit_test(TestState& state) {
    Pixel p1 = std::numeric_limits<Pixel>::max(),
          p2 = std::numeric_limits<Pixel>::min();

    state( p1.red() == std::numeric_limits<uint8_t>::max() );
    state( p2.red() == std::numeric_limits<uint8_t>::min() );
    state( (p1 - p2).red() == 255 );
    state( (p1 - p2) == 255 );
}
