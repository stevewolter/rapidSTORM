#ifndef DSTORM_ENGINE_ENGINETHREAD_H
#define DSTORM_ENGINE_ENGINETHREAD_H

#include <boost/ptr_container/ptr_vector.hpp>

#include "engine/Config.h"
#include "engine/FitPositionRoundRobin.h"
#include "engine/Input.h"
#include "engine/SingleThreadedLocalizer.h"
#include "engine/SpotFitter.h"
#include "output/LocalizedImage.h"

namespace dStorm {
namespace engine {

class EngineThread : public SingleThreadedLocalizer {
    Config& config;
    Input::TraitsPtr meta_info;

    boost::ptr_vector<spot_fitter::Implementation> fitter;
    FitPositionRoundRobin position_generator;
    int origMotivation;

    bool compute_if_enough_positions(const ImageStack& image,
                                     output::LocalizedImage* target);

  public:
    EngineThread( Config& config, Input::TraitsPtr meta_info );
    ~EngineThread() {}
    void compute(const ImageStack& image, output::LocalizedImage* target);
};

}
}

#endif
