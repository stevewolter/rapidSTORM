#include "localization_impl.h"
#include <dStorm/image/MetaInfo.h>

namespace dStorm {
namespace output {
namespace binning {

#define INSTANTIATE_WITH_LOCALIZATION_FIELD_INDEX(x) \
    template class Localization<x, IsUnscaled>; \
    template class Localization<x, Bounded>; \
    template class Localization<x, ScaledByResolution>; \
    template class Localization<x, ScaledToInterval>; \
    template class Localization<x, InteractivelyScaledToInterval>;
#include <dStorm/localization/expand.h>

}
}
}
