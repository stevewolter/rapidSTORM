#include "unit_tests.h"

namespace dStorm {
namespace fit_window {

boost::unit_test::test_suite* make_unit_test_suite() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "fit_window" );
    return rv;
    
}

}
}
