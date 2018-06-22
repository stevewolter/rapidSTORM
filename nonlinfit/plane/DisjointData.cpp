#include "nonlinfit/plane/DisjointData.h"
#include <boost/test/unit_test.hpp>

namespace nonlinfit {
namespace plane {

boost::unit_test::test_suite* register_unit_tests() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "plane" );
    return rv;
}

}
}
