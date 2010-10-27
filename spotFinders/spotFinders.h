#ifndef DSTORM_SPOTFINDERS_H
#define DSTORM_SPOTFINDERS_H

#include <memory>
#include <dStorm/engine/SpotFinder_decl.h>

namespace dStorm {
namespace spotFinders {

std::auto_ptr<engine::SpotFinderFactory> make_Spalttiefpass();
std::auto_ptr<engine::SpotFinderFactory> make_Median();
std::auto_ptr<engine::SpotFinderFactory> make_Erosion();
std::auto_ptr<engine::SpotFinderFactory> make_Gaussian();

}
}

#endif
