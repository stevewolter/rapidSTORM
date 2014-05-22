#include "engine/SingleThreadedLocalizer.h"

#include "engine/EngineThread.h"

namespace dStorm {
namespace engine {

std::unique_ptr<SingleThreadedLocalizer> SingleThreadedLocalizer::create(
    Engine& engine, Config& config, Input::TraitsPtr meta_info) {
    return std::unique_ptr<SingleThreadedLocalizer>(
        new EngineThread(engine, config, meta_info));
}

}
}
