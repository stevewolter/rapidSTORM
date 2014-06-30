#include <boost/test/unit_test.hpp>

#include "inputs/join_test.h"
#include "inputs/MedianFilter_test.h"

namespace dStorm {
namespace inputs {

boost::unit_test::test_suite* unit_test_suite() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "inputs" );
    rv->add( median_filter::unit_test_suite() );
    rv->add( join::unit_test_suite() );
    return rv;
}

}
}
