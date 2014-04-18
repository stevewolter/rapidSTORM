#include <boost/test/unit_test.hpp>

#include "inputs/MedianFilter_test.h"

namespace dStorm {
namespace inputs {

boost::unit_test::test_suite* unit_test_suite() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "guf" );
    rv->add( BOOST_TEST_CASE( &median_filter::unit_test ) );
    return rv;
}

}
}
