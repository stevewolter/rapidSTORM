#include "unit_test.h"

namespace dStorm {
namespace measured_psf {

void check_evaluator() {
    Model model;
    Evaluator<double,2> evaluator(model);
    Eigen::Array2d value;
    evaluator.value(value);
    BOOST_CHECK_CLOSE( value[0], 1E-10, 1E-2 );
}

boost::unit_test::test_suite* make_unit_test_suite() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "measured_psf" );
    rv->add( BOOST_UNIT_TEST(&check_evaluator) );
    return rv;
}
}
}
