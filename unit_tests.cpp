#include "unit_tests.h"
#include "inputs/FileMethod.h"
#include "inputs/ResolutionSetter.h"
#include "inputs/TIFF.h"
#include "dejagnu.h"
#include <dStorm/helpers/thread.h>
#include "engine/ChainLink_decl.h"
#include "guf/unit_tests.h"

namespace dStorm {
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

    return ( state.had_errors() ? EXIT_FAILURE : EXIT_SUCCESS );
}
