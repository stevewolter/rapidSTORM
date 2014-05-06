#include "estimate_psf_form/unit_test.h"
#include "estimate_psf_form/VariableReduction_test.h"

namespace dStorm {
namespace estimate_psf_form {

boost::unit_test::test_suite* test_unit_tests() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "estimate_psf_form" );
    rv->add( test_VariableReduction() );
    return rv;
}

}
}
