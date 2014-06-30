#include "engine/SingleThreadedLocalizer.h"

#include "engine/EngineThread.h"
#include "engine/PlaneSeparator.h"

namespace dStorm {
namespace engine {

std::unique_ptr<SingleThreadedLocalizer> SingleThreadedLocalizer::create(
    Config& config, Input::TraitsPtr meta_info) {
    if (config.separate_plane_fitting()) {
        return std::unique_ptr<SingleThreadedLocalizer>(
            new PlaneSeparator(config, meta_info));
    } else {
        return std::unique_ptr<SingleThreadedLocalizer>(
            new EngineThread(config, meta_info));
    }
}

}
}
