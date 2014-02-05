#ifndef DSTORM_SPOTFINDERS_H
#define DSTORM_SPOTFINDERS_H

#include <memory>
#include "engine/SpotFinder_decl.h"
#include "spotFinders/Fillhole.h"

namespace dStorm {
namespace erosion_smoother { std::auto_ptr<engine::spot_finder::Factory> make_spot_finder_factory(); }
namespace gauss_smoother { std::auto_ptr<engine::spot_finder::Factory> make_spot_finder_factory(); }
namespace median_smoother { std::auto_ptr<engine::spot_finder::Factory> make_spot_finder_factory(); }
namespace spalttiefpass_smoother { std::auto_ptr<engine::spot_finder::Factory> make_spot_finder_factory(); }
namespace spaltbandpass_smoother { std::auto_ptr<engine::spot_finder::Factory> make_spot_finder_factory(); }
}

#endif
