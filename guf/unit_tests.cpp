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

void test_DataPlane( TestState& );

void run_unit_tests( TestState& state ) {
    test_DataPlane(state);
    nonlinfit::run_unit_tests( state );
    dStorm::guf::PSF::run_unit_tests( state );
}

}
}
