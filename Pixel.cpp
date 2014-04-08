#include "Pixel.h"
#include <iostream>
#include <cassert>
#include <boost/test/unit_test.hpp>

namespace dStorm {

void pixel_unit_test() {
    Pixel p1 = std::numeric_limits<Pixel>::max(),
          p2 = std::numeric_limits<Pixel>::min();

    BOOST_CHECK_EQUAL( p1.red(), std::numeric_limits<uint8_t>::max() );
    BOOST_CHECK_EQUAL( p2.red(), std::numeric_limits<uint8_t>::min() );
    BOOST_CHECK_EQUAL( (p1 - p2).red(), 255 );
    BOOST_CHECK_EQUAL( (p1 - p2), 255 );
}

}
