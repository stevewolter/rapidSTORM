/** \cond */
#include <cassert>
#include <nonlinfit/Terminator.h>
#include "RelativeChange.h"
#include "StepLimit.h"
#include "dejagnu.h"

namespace nonlinfit {
namespace terminators {

void run_unit_tests(TestState& state) {
    state( is_terminator<terminators::RelativeChange>(), 
        "Relative change is a valid terminator" ); 
    state( is_terminator<terminators::StepLimit>(), 
        "Relative change is a valid terminator" ); 
}

}
}
/** \endcond */
