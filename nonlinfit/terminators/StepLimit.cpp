/** \cond */
#include <nonlinfit/Terminator.h>
#include "nonlinfit/terminators/StepLimit.h"

namespace nonlinfit {
namespace terminators {

void test_StepLimit()
{
    assert( is_terminator<terminators::StepLimit>() );
}

}
}
/** \endcond */
