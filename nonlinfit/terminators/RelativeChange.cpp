/** \cond */
#include <cassert>
#include <nonlinfit/Terminator.h>
#include "nonlinfit/terminators/RelativeChange.h"

namespace nonlinfit {
namespace terminators {

void test_RelativeChange() { 
    assert( (is_terminator<terminators::RelativeChange>()) ); 
}

}
}
/** \endcond */
