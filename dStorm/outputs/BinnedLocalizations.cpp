#include "BinnedLocalizations.h"
#include "BinnedLocalizations_impl.h"
#include <dStorm/ImageTraits_impl.h>
#include <dStorm/Image_impl.h>

namespace dStorm {
namespace input {
    template class Traits< Image<float,2> >;
    template class Traits< Image<float,3> >;
}

namespace outputs {
    template class BinnedLocalizations<DummyBinningListener<2>, 2>;
}
}
