#ifndef DSTORM_INPUTS_FILEMETHOD_H
#define DSTORM_INPUTS_FILEMETHOD_H

#include <dStorm/input/fwd.h>

class TestState;

namespace dStorm {
namespace input {
namespace file_method {
std::auto_ptr<Link> makeLink();
void unit_test( TestState& );
}
}
}

#endif
