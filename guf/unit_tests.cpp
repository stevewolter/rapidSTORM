#include <Eigen/StdVector>
#include <dStorm/helpers/thread.h>
#include <boost/mpl/vector.hpp>
#include "dejagnu.h"

void check_naive_fitter( TestState& );

namespace nonlinfit {
    void run_unit_tests( TestState& );
}
namespace dStorm {
namespace guf {
namespace PSF {
    void run_unit_tests( TestState& ); 
}

void run_unit_tests( TestState& state ) {
    nonlinfit::run_unit_tests( state );
    dStorm::guf::PSF::run_unit_tests( state );
}

}
}
