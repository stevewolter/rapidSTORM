#include "gaussian_psf/No3D_test.h"
#include "gaussian_psf/No3D.h"
#include "gaussian_psf/check_evaluator.hpp"

namespace dStorm {
namespace gaussian_psf {

boost::unit_test::test_suite* test_No3D() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE("No3D");
    check_evaluator<No3D>(rv);
    return rv;
}

}
}
