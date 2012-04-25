#include <Eigen/StdVector>
#include <dStorm/helpers/thread.h>
#include <boost/mpl/vector.hpp>
#include "dejagnu.h"
#include <boost/test/unit_test.hpp>

namespace nonlinfit {
    void run_unit_tests( TestState& );
}
namespace dStorm {
namespace guf {
namespace PSF {
    void run_unit_tests( TestState& ); 
}

void run_unit_tests( TestState& state ) {
    nonlinfit::run_unit_tests( state );
    dStorm::guf::PSF::run_unit_tests( state );
}

boost::unit_test::test_suite* test_unit_tests() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "guf" );
    return rv;
}

}
}
