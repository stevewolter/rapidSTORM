#ifndef DSTORM_ENGINE_THRESHOLD_GUESSER_H
#define DSTORM_ENGINE_THRESHOLD_GUESSER_H

#include <dStorm/engine/Input.h>
#include <cs_units/camera/intensity.hpp>
#include <boost/units/quantity.hpp>

namespace dStorm {
namespace engine {

class ThresholdGuesser {
    Input& input;
    const float confidence_limit;
    const int binning;

  public:
    ThresholdGuesser( Input& input );

    boost::units::quantity<cs_units::camera::intensity,float>
        compute_threshold();
};

}
}

#endif
