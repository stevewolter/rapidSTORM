#include "engine/FitPositionGenerator.h"
#include "traits/Projection.h"

namespace dStorm {
namespace engine {

FitPositionGenerator::FitPositionGenerator(
    const Config& config, const InputPlane& plane)
  : maximumLimit(20),
    maximums(1, 1, 1, 1),
    projection(plane.projection()) {
    if ( ! config.spotFindingMethod.isValid() )
        throw std::runtime_error("No spot finding method selected.");
    spot_finder::Job job(plane);
    finder = config.spotFindingMethod().make(job);

    maximums.setLimit(maximumLimit);
    current = maximums.begin();
}

void FitPositionGenerator::compute_positions(const Image2D& image) {
    finder->smooth(image);
    finder->findCandidates( maximums );
}

bool FitPositionGenerator::next_position(FitPosition* fit_position) {
    if (current == maximums.end()) {
        return false;
    } else {
        Spot::CameraPosition spot = current->spot().position();
        FitPosition position = boost::units::value(
            projection.pixel_in_sample_space(spot).head<2>())
            .cast<double>() * 1E6;
        ++current;
        return true;
    }
}

void FitPositionGenerator::extend_range() {
    maximumLimit *= 2;
    maximums.setLimit(maximumLimit);
    finder->findCandidates( maximums );
    current = maximums.begin();
}

}
}
