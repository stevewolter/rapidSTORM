#ifndef DSTORM_ENGINE_CHAINLINK_DECL_H
#define DSTORM_ENGINE_CHAINLINK_DECL_H

#include <dStorm/input/chain/Filter_decl.h>
#include <memory>

namespace dStorm {
namespace engine {

class ChainLink;

std::auto_ptr<input::chain::Filter>
make_rapidSTORM_engine_link();

}
}

#endif
