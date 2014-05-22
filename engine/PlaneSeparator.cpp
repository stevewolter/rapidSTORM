#include "engine/PlaneSeparator.h"

#include "engine/EngineThread.h"

namespace dStorm {
namespace engine {

PlaneSeparator::PlaneSeparator(
        Config& config, Input::TraitsPtr meta_info) {
    if (meta_info->fluorophore_count != 1) {
        throw std::runtime_error("Plane separation and more than one "
                "fluorophore don't go well together");
    }
    for (int plane = 0; plane < meta_info->plane_count(); ++plane) {
        Input::TraitsPtr plane_traits(new input::Traits<engine::ImageStack>(*meta_info));
        plane_traits->clear();
        plane_traits->push_back(meta_info->plane(plane));
        planes_.emplace_back(new EngineThread(config, plane_traits));
    }
}

void PlaneSeparator::compute(const ImageStack& image, output::LocalizedImage* target) {
    assert(image.plane_count() == int(planes_.size()));
    for (int plane = 0; plane < image.plane_count(); ++plane) {
        output::LocalizedImage result_part;
        ImageStack plane_image(image);
        plane_image.clear();
        plane_image.push_back(image.plane(plane));
        plane_image.set_background(0, image.background(plane));
        planes_[plane]->compute(plane_image, &result_part);
        std::copy(result_part.begin(), result_part.end(),
                  std::back_inserter(*target));
    }
}

}
}
