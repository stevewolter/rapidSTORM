#include <boost/test/unit_test.hpp>

namespace dStorm {
namespace guf {

boost::unit_test::test_suite* test_unit_tests() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "guf" );
    return rv;
}

}
}
