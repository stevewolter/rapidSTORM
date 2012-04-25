#include "unit_tests.h"

namespace dStorm {
namespace fit_window {

boost::unit_test::test_suite* test_schedule_index_finder();
boost::unit_test::test_suite* test_FittingRegionCreator();

boost::unit_test::test_suite* make_unit_test_suite() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "guf" );
    rv->add( test_schedule_index_finder() );
    rv->add( test_FittingRegionCreator() );
    return rv;
    
}

}
}
