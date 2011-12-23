#include "inputs/FileMethod.h"
#include "inputs/ResolutionSetter_decl.h"
#include "inputs/TIFF.h"
#include "dejagnu.h"
#include <dStorm/helpers/thread.h>
#include "engine/ChainLink_decl.h"

int main() {
    ost::DebugStream::set( std::cerr );
    TestState state;
    dStorm::engine::unit_test(state);
    dStorm::input::file_method::unit_test( state );
    dStorm::TIFF::unit_test( state );
    dStorm::input::resolution::unit_test(state);
    return ( state.had_errors() ? EXIT_FAILURE : EXIT_SUCCESS );
}
