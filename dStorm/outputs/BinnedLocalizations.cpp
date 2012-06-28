#include "BinnedLocalizations.h"
#include "BinnedLocalizations_impl.h"
#include <dStorm/Image_impl.h>

namespace dStorm {

namespace outputs {
    template class BinnedLocalizations<DummyBinningListener<2>, 2>;
    template class BinnedLocalizations<DummyBinningListener<3>, 3>;

}
}
