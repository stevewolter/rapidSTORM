#include "inputs/join_test.h"
#include "inputs/join.h"

namespace dStorm {
namespace inputs {
namespace join {

boost::unit_test::test_suite* unit_test_suite() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "join" );
    return rv;
}

}
}
}
