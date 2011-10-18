#ifndef DSTORM_OUTPUT_BINNING_LOCALIZATION_CONFIG_DECL_H
#define DSTORM_OUTPUT_BINNING_LOCALIZATION_CONFIG_DECL_H

#include "config.h"
#include "binning.h"

namespace dStorm {
namespace output {
namespace binning {

template <int Field>
class LocalizationConfig;

std::auto_ptr<FieldConfig> make_localization_config(int field_index, int row, int column);

}
}
}

#endif
