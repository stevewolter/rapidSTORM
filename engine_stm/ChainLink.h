#ifndef DSTORM_ENGINE_STM_CHAINLINK_DECL_H
#define DSTORM_ENGINE_STM_CHAINLINK_DECL_H

#include <memory>
#include "input/FilterFactory.h"
#include "localization/record.h"
#include "output/LocalizedImage.h"

namespace dStorm {
namespace engine_stm {

std::unique_ptr<input::FilterFactory<localization::Record, output::LocalizedImage>> create();

}
}

#endif
