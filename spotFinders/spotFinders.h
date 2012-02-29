#ifndef DSTORM_SPOTFINDERS_H
#define DSTORM_SPOTFINDERS_H

#include <memory>
#include <dStorm/engine/SpotFinder_decl.h>
#include "Fillhole.h"

namespace dStorm {
namespace spotFinders {

std::auto_ptr<engine::spot_finder::Factory> make_Spalttiefpass();
std::auto_ptr<engine::spot_finder::Factory> make_Median();
std::auto_ptr<engine::spot_finder::Factory> make_Erosion();
std::auto_ptr<engine::spot_finder::Factory> make_Gaussian();

}
}

#endif
