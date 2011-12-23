#ifndef DSTORM_INPUTS_FILEMETHOD_H
#define DSTORM_INPUTS_FILEMETHOD_H

#include <dStorm/input/chain/Link_decl.h>

class TestState;

namespace dStorm {
namespace input {
namespace file_method {
std::auto_ptr<chain::Link> makeLink();
void unit_test( TestState& );
}
}
}

#endif
