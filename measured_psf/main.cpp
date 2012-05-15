#define BOOST_TEST_ALTERNATIVE_INIT_API
#include "measured_psf/unit_test.h"
#include <boost/test/unit_test.hpp>

bool init_unit_test() {
    boost::unit_test::framework::master_test_suite().
        add( dStorm::measured_psf::make_unit_test_suite() );
    return true;
}

int main(int argc, char* argv[]) {
    return ::boost::unit_test::unit_test_main( &init_unit_test, argc, argv );
}
