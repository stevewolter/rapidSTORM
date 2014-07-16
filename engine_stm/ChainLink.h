#ifndef DSTORM_ENGINE_STM_CHAINLINK_DECL_H
#define DSTORM_ENGINE_STM_CHAINLINK_DECL_H

#include "input/fwd.h"
#include <memory>

namespace dStorm {
namespace engine_stm {

std::auto_ptr<input::Link> make_localization_buncher();

}
}

#endif
