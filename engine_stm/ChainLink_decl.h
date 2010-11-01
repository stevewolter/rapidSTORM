#ifndef DSTORM_ENGINE_STM_CHAINLINK_DECL_H
#define DSTORM_ENGINE_STM_CHAINLINK_DECL_H

#include <dStorm/input/chain/Filter_decl.h>
#include <memory>

namespace dStorm {
namespace engine_stm {

class ChainLink;

std::auto_ptr<input::chain::Filter>
make_STM_engine_link();

}
}

#endif
