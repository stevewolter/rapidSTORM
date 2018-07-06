#include <boost/test/unit_test.hpp>

namespace dStorm {
namespace image {

void iterator_unit_test();
void slice_unit_test();
void reconstruction_by_dilation_unit_test();
void box_unit_test();

boost::unit_test::test_suite* unit_test_suite() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "image" );
    rv->add( BOOST_TEST_CASE( &iterator_unit_test ) );
    rv->add( BOOST_TEST_CASE( &slice_unit_test ) );
    rv->add( BOOST_TEST_CASE( &reconstruction_by_dilation_unit_test ) );
    rv->add( BOOST_TEST_CASE( &box_unit_test ) );
    return rv;
}

}
}

bool init_unit_tests() {
    boost::unit_test::framework::master_test_suite().
        add(dStorm::image::unit_test_suite());
    return true;
}

int main(int argc, char* argv[]) {
    return ::boost::unit_test::unit_test_main( &init_unit_tests, argc, argv );
}
