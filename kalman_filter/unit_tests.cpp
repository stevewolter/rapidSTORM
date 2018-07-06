#include <boost/test/unit_test.hpp>
#include "kalman_filter/EmissionTracker_test.h"

bool init_unit_tests() {
    boost::unit_test::framework::master_test_suite().
        add( dStorm::kalman_filter::emission_tracker::test_suite() );
    return true;
}

int main(int argc, char* argv[]) {
    return ::boost::unit_test::unit_test_main( &init_unit_tests, argc, argv );
}

