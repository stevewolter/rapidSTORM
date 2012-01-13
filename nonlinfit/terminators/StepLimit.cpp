/** \cond */
#include <nonlinfit/Terminator.h>
#include "StepLimit.h"

namespace nonlinfit {
namespace terminators {

void test_StepLimit()
{
    assert( is_terminator<terminators::StepLimit>() );
}

}
}
/** \endcond */
