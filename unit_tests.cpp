#define BOOST_TEST_ALTERNATIVE_INIT_API

#include "unit_tests.h"

#include <boost/test/unit_test.hpp>

#include "alignment_fitter.h"
#include "dejagnu.h"
#include "engine/ChainLink.h"
#include "estimate_psf_form/unit_test.h"
#include "fit_window/unit_tests.h"
#include "gaussian_psf/unit_test.h"
#include "guf/unit_tests.h"
#include "helpers/thread.h"
#include "nonlinfit/unit_test.h"
#include "simparm/text_stream/unit_tests.h"
#include "simparm/unit_tests.h"
#include "threed_info/fwd.h"
#include "tiff/TIFF.h"

namespace dStorm {

void pixel_unit_test();

namespace expression {

void unit_test( TestState& );

}
}

bool init_unit_test() {
    boost::unit_test::framework::master_test_suite().
        add( dStorm::guf::test_unit_tests() );
    boost::unit_test::framework::master_test_suite().
        add( dStorm::fit_window::make_unit_test_suite() );
    boost::unit_test::framework::master_test_suite().
        add( dStorm::gaussian_psf::make_unit_test_suite() );
    boost::unit_test::framework::master_test_suite().
        add( nonlinfit::register_unit_tests() );
    boost::unit_test::framework::master_test_suite().
        add( dStorm::tiff::register_unit_tests() );
    boost::unit_test::framework::master_test_suite().
        add( simparm::text_stream::make_unit_tests() );
    boost::unit_test::framework::master_test_suite().
        add( simparm::test_unit_tests() );
    boost::unit_test::framework::master_test_suite().
        add( register_alignment_fitter_unit_tests() );
    boost::unit_test::framework::master_test_suite().
        add( dStorm::estimate_psf_form::test_unit_tests() );

    boost::unit_test::framework::master_test_suite().
        add( BOOST_TEST_CASE( &dStorm::pixel_unit_test ) );

    return true;
}

int run_unit_tests(int argc, char* argv[]) {
    TestState state;
    dStorm::engine::unit_test(state);
    dStorm::expression::unit_test( state );
    dStorm::threed_info::unit_tests( state );

    int success = ::boost::unit_test::unit_test_main( &init_unit_test, argc, argv );

    return ( (state.had_errors() || success != 0) ? EXIT_FAILURE : EXIT_SUCCESS );
}
