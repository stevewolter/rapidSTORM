#include "BinnedLocalizations.h"
#include "BinnedLocalizations_impl.h"
#include <dStorm/ImageTraits_impl.h>
#include <dStorm/Image_impl.h>

namespace dStorm {
namespace input {
    template class Traits< outputs::BinnedImage >;
}

namespace outputs {
    template class BinnedLocalizations<DummyBinningListener>;
}
}
