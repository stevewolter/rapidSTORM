#ifndef DSTORM_INPUT_JOIN_H
#define DSTORM_INPUT_JOIN_H

#include "input/Link.h"
#include <memory>

namespace dStorm {
namespace inputs {
namespace join {

std::auto_ptr<input::Link> create_link();

}
}
}

#endif
