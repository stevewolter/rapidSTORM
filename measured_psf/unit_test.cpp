#include "unit_test.h"

namespace dStorm {
namespace measured_psf {

struct No3D;
struct Polynomial3D;
struct Spline3D;

template <typename Model>
boost::unit_test::test_suite* check_evaluator(const char *);

boost::unit_test::test_suite* make_unit_test_suite() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "gaussian_psf" );
    rv->add( check_evaluator<No3D>("No3D") );
    rv->add( check_evaluator<Polynomial3D>( "Polynomial3D" ) );
    rv->add( check_evaluator<Spline3D>("Spline3D") );
    return rv;
}
    
}
}
