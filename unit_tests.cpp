#include "unit_tests.h"
#include "inputs/FileMethod.h"
#include "inputs/ResolutionSetter.h"
#include "inputs/TIFF.h"
#include "dejagnu.h"
#include <dStorm/helpers/thread.h>
#include "engine/ChainLink_decl.h"
#include "guf/unit_tests.h"
#include <dStorm/traits/unit_tests.h>
#include "viewer/fwd.h"

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

int run_unit_tests() {
    TestState state;
    dStorm::engine::unit_test(state);
    dStorm::input::file_method::unit_test( state );
    dStorm::TIFF::unit_test( state );
    dStorm::input::resolution::unit_test(state);
    dStorm::guf::run_unit_tests(state);
    dStorm::expression::unit_test( state );
    dStorm::viewer::unit_test( state );
    dStorm::traits::run_unit_tests( state );
    pixel_unit_test( state );
    locprec::run_unit_tests( state );
    dStorm::image_unit_tests( state );

    return ( state.had_errors() ? EXIT_FAILURE : EXIT_SUCCESS );
}
