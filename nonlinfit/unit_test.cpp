/** \cond */
#include "dejagnu.h"
namespace nonlinfit {

namespace sum { void run_unit_tests(TestState&); }
namespace levmar { void run_unit_tests(TestState&); }
namespace terminators { void run_unit_tests(TestState&); }
namespace plane { void run_unit_tests(TestState&); }

void run_unit_tests(TestState& state) {
    sum::run_unit_tests(state);
    levmar::run_unit_tests(state);
    terminators::run_unit_tests(state);
    plane::run_unit_tests(state);
}

}
/** \endcond */
