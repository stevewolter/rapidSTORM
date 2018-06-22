#include "gaussian_psf/unit_test.h"
#include "gaussian_psf/DepthInfo3D_test.h"
#include "gaussian_psf/No3D_test.h"

namespace dStorm {
namespace gaussian_psf {

boost::unit_test::test_suite* make_unit_test_suite() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE( "gaussian_psf" );
    rv->add( test_No3D() );
    rv->add( test_DepthInfo3D() );
    return rv;
}
    
}
}
