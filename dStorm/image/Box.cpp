#include "dStorm/image/Box.hpp"
#include <boost/test/unit_test.hpp>

namespace dStorm {
namespace image {

template class Box<2>;
template class Box<3>;

void box_unit_test() {
    Box<3> test_box( Box<3>::Position::Constant( 2 * camera::pixel ), Box<3>::Position::Constant( 5 * camera::pixel ) );
    BOOST_CHECK_EQUAL( test_box.width(Direction_X), 4 * camera::pixel );
    BOOST_CHECK_EQUAL( test_box.volume(), pow<3>(4 * camera::pixel) );

    BOOST_CHECK_EQUAL( Box<3>::ZeroOrigin( Box<3>::Position::Constant( 3 * camera::pixel ) ).volume(), pow<3>(3*camera::pixel) );
}

}
}
