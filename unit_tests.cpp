#define BOOST_TEST_ALTERNATIVE_INIT_API

#include "unit_tests.h"
#include "inputs/FileMethod.h"
#include "inputs/ResolutionSetter.h"
#include "tiff/TIFF.h"
#include "dejagnu.h"
#include <dStorm/helpers/thread.h>
#include "engine/ChainLink_decl.h"
#include "guf/unit_tests.h"
#include <dStorm/traits/unit_tests.h>
#include <dStorm/threed_info/fwd.h>
#include <boost/test/unit_test.hpp>
#include "fit_window/unit_tests.h"
#include "gaussian_psf/unit_test.h"
#include "nonlinfit/unit_test.h"
#include "simparm/text_stream/unit_tests.h"
#include "simparm/unit_tests.h"

void pixel_unit_test(TestState& state);

namespace locprec {
void run_unit_tests( TestState& );
}

namespace dStorm {

void image_unit_tests( TestState& state );

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

    return true;
}

int run_unit_tests(int argc, char* argv[]) {
    TestState state;
    dStorm::engine::unit_test(state);
    dStorm::input::file_method::unit_test( state );
    dStorm::input::resolution::unit_test(state);
    dStorm::expression::unit_test( state );
    dStorm::traits::run_unit_tests( state );
    pixel_unit_test( state );
    locprec::run_unit_tests( state );
    dStorm::image_unit_tests( state );
    dStorm::threed_info::unit_tests( state );

    int success = ::boost::unit_test::unit_test_main( &init_unit_test, argc, argv );

    return ( (state.had_errors() || success != 0) ? EXIT_FAILURE : EXIT_SUCCESS );
}
