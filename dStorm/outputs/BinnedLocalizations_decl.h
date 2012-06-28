#ifndef DSTORM_OUTPUTS_BINNEDLOCALIZATIONS_DECL_H
#define DSTORM_OUTPUTS_BINNEDLOCALIZATIONS_DECL_H

#include "../image/fwd.h"

namespace dStorm {
namespace outputs {

template <int Dimensions> struct BinningStrategy;
template <typename KeepUpdated, int Dimensions> class BinnedLocalizations;

}
}

#endif
