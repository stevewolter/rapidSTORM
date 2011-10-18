#include "localization_impl.h"
#include <dStorm/ImageTraits_impl.h>

namespace dStorm {
namespace output {
namespace binning {

#define INSTANTIATE_WITH_LOCALIZATION_FIELD_INDEX(x) \
    template class Localization<x, IsUnscaled, false>; \
    template class Localization<x, Bounded, false>; \
    template class Localization<x, ScaledByResolution, false>; \
    template class Localization<x, ScaledToInterval, false>; \
    template class Localization<x, InteractivelyScaledToInterval, false>;
#include <dStorm/localization/expand.h>

}
}
}
