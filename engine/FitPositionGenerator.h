#ifndef DSTORM_ENGINE_FITPOSITION_GENERATOR_H
#define DSTORM_ENGINE_FITPOSITION_GENERATOR_H

#include "engine/CandidateTree.h"
#include "engine/Config.h"
#include "engine/FitPosition.h"
#include "engine/InputPlane.h"
#include "engine/SpotFinder.h"

namespace dStorm {
namespace engine {

class FitPositionGenerator {
  public:
    FitPositionGenerator(const Config& config,
                         const InputPlane& plane);
    void compute_positions(const Image2D& image);
    bool next_position(FitPosition* fit_position);
    void extend_range();
    bool reached_size_limit() const { return maximums.reached_size_limit(); }

  private:
    int maximumLimit;
    const traits::Projection& projection;
    std::unique_ptr<spot_finder::Base> finder;
    CandidateTree<SmoothedPixel> maximums;
    CandidateTree<SmoothedPixel>::const_iterator current;
};

}
}

#endif
