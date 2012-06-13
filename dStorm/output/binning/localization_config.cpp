#include "localization_config_impl.h"

namespace dStorm {
namespace output {
namespace binning {


#define INSTANTIATE_WITH_LOCALIZATION_FIELD_INDEX(x) \
    template class LocalizationConfig<x>;
#include <dStorm/localization/expand.h>

}
}
}
