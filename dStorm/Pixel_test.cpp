#ifdef NDEBUG
#undef NDEBUG
#endif
#include "Pixel.h"
#include <iostream>
#include <cassert>

using namespace dStorm;

int main() {
    Pixel p1 = std::numeric_limits<Pixel>::max(),
          p2 = std::numeric_limits<Pixel>::min();

    assert( p1.red() == std::numeric_limits<uint8_t>::max() );
    assert( p2.red() == std::numeric_limits<uint8_t>::min() );
    assert( (p1 - p2).red() == 255 );
    assert( (p1 - p2) == 255 );
}
