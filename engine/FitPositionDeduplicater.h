#ifndef DSTORM_ENGINE_FITPOSITION_DEDUPLICATOR_H
#define DSTORM_ENGINE_FITPOSITION_DEDUPLICATOR_H

#include <vector>

#include "engine/FitPositionRoundRobin.h"
#include "engine/InputTraits.h"
#include "engine/SpotFinder.h"
#include "image/subtract.hpp"

namespace dStorm {
namespace engine {

class FitPositionDeduplicater {
  public:
    FitPositionDeduplicater(const Config& config, const InputTraits& traits)
        : epsilon_(config.fit_position_epsilon() / boost::units::si::nanometre / 1E3),
          round_robin_(config, traits) {}

    void compute_positions(const ImageStack& image) {
        fitted_positions_.clear();
        round_robin_.compute_positions(image);
    }

    bool next_position(FitPosition* fit_position) {
        bool found_duplicate;
        do {
            if (!round_robin_.next_position(fit_position)) {
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
        fitted_positions_.clear();
        round_robin_.extend_range();
    }

    bool reached_size_limit() const {
        return round_robin_.reached_size_limit();
    }

  private:
    const double epsilon_;
    FitPositionRoundRobin round_robin_;
    std::vector<FitPosition> fitted_positions_;
};

}
}

#endif
