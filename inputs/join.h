#ifndef DSTORM_INPUT_JOIN_H
#define DSTORM_INPUT_JOIN_H

#include "input/Link.h"
#include <memory>

namespace dStorm {
namespace inputs {
namespace join {

std::unique_ptr<input::Link> create_link();

}
}
}

#endif
