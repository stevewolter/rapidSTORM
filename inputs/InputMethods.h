#ifndef DSTORM_INPUT_INPUTMETHODS_H
#define DSTORM_INPUT_INPUTMETHODS_H

#include <memory>
#include "input/fwd.h"

namespace dStorm {
namespace inputs {
namespace InputMethods {

std::auto_ptr<input::Link> create();

}
}
}

#endif
