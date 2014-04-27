/** \cond */
#include <boost/test/unit_test.hpp>
namespace nonlinfit {

namespace sum { boost::unit_test::test_suite* register_unit_tests(); }
namespace levmar { boost::unit_test::test_suite* register_unit_tests(); }

boost::unit_test::test_suite* register_unit_tests() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "nonlinfit" );
    rv->add( sum::register_unit_tests() );
    rv->add( levmar::register_unit_tests() );
    return rv;
}

}
/** \endcond */
