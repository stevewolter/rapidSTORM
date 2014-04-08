#ifndef DSTORM_ENGINE_CHAINLINK_DECL_H
#define DSTORM_ENGINE_CHAINLINK_DECL_H

#include "input/fwd.h"
#include <memory>

class TestState;

namespace dStorm {
namespace engine {

class ChainLink;

std::auto_ptr<input::Link>
make_rapidSTORM_engine_link();
void unit_test( TestState& );

}
}

#endif
