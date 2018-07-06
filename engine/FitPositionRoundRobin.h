#ifndef DSTORM_ENGINE_FITPOSITION_ROUNDROBIN_H
#define DSTORM_ENGINE_FITPOSITION_ROUNDROBIN_H

#include <vector>

#include "engine/FitPositionGenerator.h"
#include "engine/InputTraits.h"
#include "engine/SpotFinder.h"
#include "image/subtract.hpp"

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
            Image2D signal = image.background(plane).is_valid() ?
                subtract(image.plane(plane), image.background(plane)) :
                image.plane(plane);
            generators_[plane]->compute_positions(signal);
        }
    }

    bool next_position(FitPosition* fit_position) {
        for (int ignored = 0; ignored < int(generators_.size()); ++ignored) {
            FitPositionGenerator* generator =
                generators_[current_plane_ % generators_.size()].get();
            ++current_plane_;
            if (generator->next_position(fit_position)) {
                return true;
            } else if (generator->reached_size_limit()) {
                return false;
            }
        }

        return false;
    }
    void extend_range() {
        current_plane_ = 0;
        for (auto& generator : generators_) {
            generator->extend_range();
        }
    }

    bool reached_size_limit() const {
        for (auto& generator : generators_) {
            if (generator->reached_size_limit()) {
                return true;
            }
        }
        return false;
    }

  private:
    int current_plane_;
    std::vector<std::unique_ptr<FitPositionGenerator>> generators_;

};

}
}

#endif
