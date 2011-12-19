#ifndef DSTORM_NOOP_ENGINE_CHAINLINK_DECL_H
#define DSTORM_NOOP_ENGINE_CHAINLINK_DECL_H

#include <dStorm/input/chain/Link_decl.h>
#include <memory>

namespace dStorm {
namespace noop_engine {

class ChainLink;

std::auto_ptr<input::chain::Link>
makeLink();

}
}

#endif
