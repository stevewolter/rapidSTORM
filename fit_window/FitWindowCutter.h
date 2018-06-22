#ifndef FIT_WINDOW_CUT_REGION_OF_INTEREST_H
#define FIT_WINDOW_CUT_REGION_OF_INTEREST_H

#include "engine/InputTraits.h"
#include "fit_window/Config.h"
#include "fit_window/Plane.h"
#include "fit_window/Optics.h"

namespace dStorm {
namespace fit_window {

class FitWindowCutter {
  public:
    FitWindowCutter(const Config& c, const dStorm::engine::InputTraits& traits,
            std::set<int> desired_fit_window_widths, int fit_window_width_slack);

    std::vector<Plane> cut_region_of_interest(
        const dStorm::engine::ImageStack& image,
        const Spot& position);

  private:
    std::vector<Optics> optics;
    std::set<int> desired_fit_window_widths;
    int fit_window_width_slack;

    Plane cut_region_of_interest(
        const Optics& optics,
        const dStorm::engine::Image2D& image,
        const Spot& position);
};

}
}

#endif
