#ifndef DSTORM_OUTPUTS_BINNEDLOCALIZATIONS_DECL_H
#define DSTORM_OUTPUTS_BINNEDLOCALIZATIONS_DECL_H

#include "../image/fwd.h"

namespace dStorm {
namespace outputs {

template <int Dimensions> class BinningListener;
template <int Dimensions> class DummyBinningListener;
template <int Dimensions, typename Listener> class BinningPublisher;
template <int Dimensions> struct BinningStrategy;
template <typename KeepUpdated = DummyBinningListener<2>, int Dimensions = 2>
class BinnedLocalizations;

}
}

#endif
