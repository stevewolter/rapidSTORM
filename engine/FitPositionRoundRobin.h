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
    FitPositionRoundRobin(const Config& config, const InputTraits& traits)
        : epsilon_(config.fit_position_epsilon() / boost::units::si::nanometre / 1E3)  {
        for (const engine::InputPlane& plane : traits) {
            generators_.emplace_back(new FitPositionGenerator(config, plane));
        }
    }

    void compute_positions(const ImageStack& image) {
        current_plane_ = 0;
        fitted_positions_.clear();
        for (int plane = 0; plane < image.plane_count(); ++plane) {
            generators_[plane]->compute_positions(image.plane(plane));
        }
    }

    bool next_position(FitPosition* fit_position) {
        bool found_duplicate;
        do {
            if (!generators_[current_plane_++ % generators_.size()]
                ->next_position(fit_position)) {
                return false;
            }
            
            found_duplicate = false;
            for (const FitPosition& previous : fitted_positions_) {
                if ((previous - *fit_position).squaredNorm() < epsilon_ * epsilon_) {
                    found_duplicate = true;
                    break;
                }
            }
        } while (found_duplicate);

        fitted_positions_.push_back(*fit_position);
        return true;
    }

    void extend_range() {
        current_plane_ = 0;
        fitted_positions_.clear();
        for (auto& generator : generators_) {
            generator->extend_range();
        }
    }

  private:
    const double epsilon_;
    int current_plane_;
    std::vector<std::unique_ptr<FitPositionGenerator>> generators_;
    std::vector<FitPosition> fitted_positions_;
};

}
}

#endif
