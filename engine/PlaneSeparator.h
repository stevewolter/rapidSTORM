#ifndef DSTORM_ENGINE_PLANE_SEPARATOR_H
#define DSTORM_ENGINE_PLANE_SEPARATOR_H

#include "engine/Config.h"
#include "output/LocalizedImage.h"
#include "engine/SingleThreadedLocalizer.h"

namespace dStorm {
namespace engine {

class PlaneSeparator : public SingleThreadedLocalizer {
    std::vector<std::unique_ptr<SingleThreadedLocalizer>> planes_;

  public:
    PlaneSeparator(Config& config, Input::TraitsPtr meta_info);
    void compute(const ImageStack& image, output::LocalizedImage* target);
};

}
}

#endif
