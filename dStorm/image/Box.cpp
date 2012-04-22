#include "Box.hpp"
#include "dejagnu.h"

namespace dStorm {
namespace image {

template class Box<2>;
template class Box<3>;

void check_box( TestState& state ) {
    Box<3> test_box( Box<3>::Position::Constant( 2 * camera::pixel ), Box<3>::Position::Constant( 5 * camera::pixel ) );
    state( test_box.width(Direction_X) == 4 * camera::pixel, "Box width is correct" );
    state( test_box.volume() == pow<3>(4 * camera::pixel), "Box volume is correct" );

    state( Box<3>::ZeroOrigin( Box<3>::Position::Constant( 3 * camera::pixel ) ).volume() == pow<3>(3*camera::pixel),
        "Zero-Origin-Box volume is correct" );
}

}
}
