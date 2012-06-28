#include "BinnedLocalizations.h"
#include "BinnedLocalizations_impl.h"
#include <dStorm/Image_impl.h>
#include "density_map/DummyListener.h"

namespace dStorm {

namespace outputs {
    template class BinnedLocalizations< density_map::DummyListener<2>, 2>;
    template class BinnedLocalizations< density_map::DummyListener<3>, 3>;

}
}
