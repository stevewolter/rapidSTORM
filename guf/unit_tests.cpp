#include <Eigen/StdVector>
#include <dStorm/helpers/thread.h>
#include <boost/mpl/vector.hpp>
#include "dejagnu.h"

namespace nonlinfit {
    void run_unit_tests( TestState& );
}
namespace dStorm {
namespace guf {
namespace PSF {
    void run_unit_tests( TestState& ); 
}
}
}

int main( int argc, char *argv[] ) {
    ost::DebugStream::set( std::cerr );
    TestState state;
    nonlinfit::run_unit_tests( state );
    dStorm::guf::PSF::run_unit_tests( state );
    return ( state.had_errors() ? EXIT_FAILURE : EXIT_SUCCESS );
}
