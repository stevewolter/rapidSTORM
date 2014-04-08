#ifndef DSTORM_SIGNALS_USE_SPOT_FINDER_H
#define DSTORM_SIGNALS_USE_SPOT_FINDER_H

#include "engine/SpotFinder_decl.h"
#include <boost/signals2/slot.hpp>
#include <boost/signals2/signal.hpp>

namespace dStorm {
namespace signals {

class UseSpotFinder
: public boost::signals2::signal< void (const engine::spot_finder::Factory&) > {};

}
}

#endif
