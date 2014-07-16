#ifndef DSTORM_ENGINE_CHAINLINK_H
#define DSTORM_ENGINE_CHAINLINK_H

#include <memory>

#include "input/FilterFactory.h"
#include "output/LocalizedImage.h"

class TestState;

namespace dStorm {
namespace engine {

std::unique_ptr<input::FilterFactory<engine::ImageStack, output::LocalizedImage>> make_rapidSTORM_engine_link();
void unit_test( TestState& );

}
}

#endif
