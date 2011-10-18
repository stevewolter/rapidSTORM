#ifndef DSTORM_OUTPUTS_BINNEDLOCALIZATIONS_DECL_H
#define DSTORM_OUTPUTS_BINNEDLOCALIZATIONS_DECL_H

#include "../Image_decl.h"

namespace dStorm {
namespace outputs {

typedef dStorm::Image<float,2> BinnedImage;
class BinningListener;
class DummyBinningListener;
template <typename Listener> class BinningPublisher;
struct BinningStrategy;
template <typename KeepUpdated = DummyBinningListener>
class BinnedLocalizations;

}
}

#endif
