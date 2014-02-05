/** \cond */
#include <cassert>
#include <nonlinfit/Terminator.h>
#include "nonlinfit/terminators/RelativeChange.h"
#include "nonlinfit/terminators/StepLimit.h"
#include <boost/test/unit_test.hpp>
#include <boost/mpl/vector.hpp>

namespace nonlinfit {
namespace terminators {

BOOST_TEST_CASE_TEMPLATE_FUNCTION( check_terminatorness, Terminator ) {
    BOOST_CHECK( is_terminator<Terminator>() );
}

boost::unit_test::test_suite* register_unit_tests() {
    typedef boost::mpl::vector< terminators::RelativeChange, terminators::StepLimit > Terminators;
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "terminators" );
    rv->add( BOOST_TEST_CASE_TEMPLATE( check_terminatorness, Terminators ) );
    return rv;
}

}
}
/** \endcond */
