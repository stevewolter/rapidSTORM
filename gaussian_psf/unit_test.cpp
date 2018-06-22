#include "gaussian_psf/unit_test.h"

namespace dStorm {
namespace gaussian_psf {

struct No3D;
struct DepthInfo3D;

template <typename Model>
boost::unit_test::test_suite* check_evaluator(const char *);

boost::unit_test::test_suite* make_unit_test_suite() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "gaussian_psf" );
    rv->add( check_evaluator<No3D>("No3D") );
    rv->add( check_evaluator<DepthInfo3D>("DepthInfo3D") );
    return rv;
}
    
}
}
