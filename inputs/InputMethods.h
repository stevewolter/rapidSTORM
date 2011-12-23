#ifndef DSTORM_INPUT_INPUTMETHODS_H
#define DSTORM_INPUT_INPUTMETHODS_H

#include <memory>
#include <dStorm/input/chain/Link_decl.h>

namespace dStorm {
namespace inputs {
namespace InputMethods {

std::auto_ptr<input::chain::Link> create();

}
}
}

#endif
