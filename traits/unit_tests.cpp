#include "dejagnu.h"

namespace dStorm {
namespace traits {

void run_support_point_projection_unit_tests( TestState& state );

void run_unit_tests( TestState& state ) {
    run_support_point_projection_unit_tests( state );
}

}
}
