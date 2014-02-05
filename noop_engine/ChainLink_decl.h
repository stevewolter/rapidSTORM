#ifndef DSTORM_NOOP_ENGINE_CHAINLINK_DECL_H
#define DSTORM_NOOP_ENGINE_CHAINLINK_DECL_H

#include "input/fwd.h"
#include <memory>

namespace dStorm {
namespace noop_engine {

class ChainLink;

std::auto_ptr<input::Link>
makeLink();

}
}

#endif
