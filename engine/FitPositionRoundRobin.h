#ifndef DSTORM_ENGINE_FITPOSITION_ROUNDROBIN_H
#define DSTORM_ENGINE_FITPOSITION_ROUNDROBIN_H

#include <vector>

#include "engine/FitPositionGenerator.h"
#include "engine/InputTraits.h"
#include "engine/SpotFinder.h"

namespace dStorm {
namespace engine {

class FitPositionRoundRobin {
  public:
    FitPositionRoundRobin(const Config& config, const InputTraits& traits) {
        for (const engine::InputPlane& plane : traits) {
            generators_.emplace_back(new FitPositionGenerator(config, plane));
        }
    }

    void compute_positions(const ImageStack& image) {
        current_plane_ = 0;
        for (int plane = 0; plane < image.plane_count(); ++plane) {
            generators_[plane]->compute_positions(image.plane(plane));
        }
    }

    bool next_position(FitPosition* fit_position) {
        return generators_[current_plane_++ % generators_.size()]
            ->next_position(fit_position);
    }

    void extend_range() {
        current_plane_ = 0;
        for (auto& generator : generators_) {
            generator->extend_range();
        }
    }

  private:
    int current_plane_;
    std::vector<std::unique_ptr<FitPositionGenerator>> generators_;
};

}
}

#endif
