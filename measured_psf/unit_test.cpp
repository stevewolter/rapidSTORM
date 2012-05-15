#include "unit_test.h"
#include "Model.h"
#include "Evaluator.h"

namespace dStorm {
namespace measured_psf {

void check_evaluator() {
    Model model = Model::mock();
    Evaluator<double,2> evaluator(model); //chunk_size 2
    Eigen::Array2d value;
    Eigen::Array<double,2,2> m;
    m.fill(4);
    evaluator.prepare_chunk(m);
    evaluator.value(value);
    BOOST_CHECK_CLOSE( value[0], 3, 1E-2 );
}

boost::unit_test::test_suite* make_unit_test_suite() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "measured_psf" );
    rv->add( BOOST_TEST_CASE(&check_evaluator) );
    return rv;
}
}
}
