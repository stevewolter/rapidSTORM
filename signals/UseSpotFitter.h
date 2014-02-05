#ifndef DSTORM_SIGNALS_USE_SPOT_FITTER_H
#define DSTORM_SIGNALS_USE_SPOT_FITTER_H

#include "engine/SpotFitterFactory_decl.h"
#include <boost/signals2/slot.hpp>
#include <boost/signals2/signal.hpp>

namespace dStorm {
namespace signals {

class UseSpotFitter
: public boost::signals2::signal< void (const engine::spot_fitter::Factory&) > {};

}
}

#endif
