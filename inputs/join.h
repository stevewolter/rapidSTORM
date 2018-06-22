#ifndef DSTORM_INPUT_JOIN_H
#define DSTORM_INPUT_JOIN_H

#include "input/Link.h"
#include <memory>

namespace dStorm {
namespace input {
namespace join {

std::auto_ptr<Link> create_link();

}
}
}

#endif
