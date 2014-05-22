#ifndef DSTORM_ENGINE_SINGLE_THREADED_LOCALIZER_H
#define DSTORM_ENGINE_SINGLE_THREADED_LOCALIZER_H

#include "engine/Config.h"
#include "engine/Input.h"
#include "output/LocalizedImage.h"

namespace dStorm {
namespace engine {

class Engine;

class SingleThreadedLocalizer {
  public:
    static std::unique_ptr<SingleThreadedLocalizer> create(
        Engine& engine, Config& config, Input::TraitsPtr meta_info);
    virtual ~SingleThreadedLocalizer() {}
    virtual void compute(const ImageStack& image, output::LocalizedImage* target) = 0;
};

}
}

#endif
