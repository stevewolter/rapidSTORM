#include "unit_test.h"
#include <nonlinfit/plane/GenericData.h>
#include "Model.h"
#include "Evaluator.h"

namespace dStorm {
namespace measured_psf {

void check_evaluator( double point, double expected_value ) {
    const int ChunkSize = 2;
    Model model = Model::mock();
    Evaluator<double,ChunkSize> evaluator(model);

    nonlinfit::plane::GenericData< LengthUnit > data;
    evaluator.prepare_iteration(data);

    Eigen::Array<double,ChunkSize,2> m;
    m.fill(point);
    evaluator.prepare_chunk(m);

    Eigen::Array2d value;
    evaluator.value(value);

    BOOST_CHECK_CLOSE( value[0], expected_value, 1E-2 );
    evaluator.derivative(value.col(0), nonlinfit::Xs<0,LengthUnit>() );
    BOOST_CHECK_CLOSE( value[0], 20.0, 1E-2 );
    evaluator.derivative(value.col(0), Mean<0>() );
    BOOST_CHECK_CLOSE( value[0], -20.0, 1E-2 );
    evaluator.derivative(value.col(0), Amplitude() );
    BOOST_CHECK_CLOSE( value[0], expected_value / 2.5, 1E-2 );
    evaluator.derivative(value.col(0), Prefactor() );
    BOOST_CHECK_CLOSE( value[0], expected_value / 2, 1E-2 );
}

void check_evaluator_even_point() {
    check_evaluator( 4, 120.0 );
}

void check_evaluator_uneven_point() {
    check_evaluator( 4.33333 , 27.3333 * 5.0 );
}

boost::unit_test::test_suite* make_unit_test_suite() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "measured_psf" );
    rv->add( BOOST_TEST_CASE(&check_evaluator_even_point) );
    rv->add( BOOST_TEST_CASE(&check_evaluator_uneven_point) );
    return rv;
}
}
}
