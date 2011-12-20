#ifndef DSTORM_ENGINE_CLASSICENGINE_H
#define DSTORM_ENGINE_CLASSICENGINE_H

#include "../input/chain/Forwarder.h"
#include "SpotFinder.h"
#include "SpotFitterFactory.h"

namespace dStorm {
namespace engine {

class ClassicEngine : public input::chain::Forwarder {
  public:
    virtual void add_spot_finder( spot_finder::Factory& ) = 0;
    virtual void add_spot_fitter( spot_fitter::Factory& ) = 0;
};

}
}

#endif
