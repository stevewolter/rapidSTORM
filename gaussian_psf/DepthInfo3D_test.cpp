#include "gaussian_psf/DepthInfo3D_test.h"
#include "gaussian_psf/DepthInfo3D.h"
#include "gaussian_psf/check_evaluator.hpp"

namespace dStorm {
namespace gaussian_psf {

static void test_termination() {
    DepthInfo3D d;
    d.set_negligible_step_length(10);
    d.set_relative_epsilon(0.5);

    BOOST_CHECK(d.change_is_negligible(Mean<1>(), 1, 9));
    BOOST_CHECK(!d.change_is_negligible(Mean<1>(), 101, 112));
    BOOST_CHECK(d.change_is_negligible(Mean<2>(), 1, 9));
    BOOST_CHECK(!d.change_is_negligible(Mean<2>(), 101, 112));
    BOOST_CHECK(!d.change_is_negligible(Amplitude(), 1, 9));
    BOOST_CHECK(d.change_is_negligible(Amplitude(), 101, 112));
}

boost::unit_test::test_suite* test_DepthInfo3D() {
    boost::unit_test::test_suite* rv = BOOST_TEST_SUITE("DepthInfo3D");
    check_evaluator<DepthInfo3D>(rv);
    rv->add( BOOST_TEST_CASE(&test_termination) );
    return rv;
}

}
}
