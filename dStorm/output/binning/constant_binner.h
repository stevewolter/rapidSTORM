#ifndef DSTORM_OUTPUT_BINNING_CONSTANT_H
#define DSTORM_OUTPUT_BINNING_CONSTANT_H

#include "binning_decl.h"
#include "config.h"
#include <memory>

namespace dStorm {
namespace output {
namespace binning {

std::auto_ptr<Unscaled> make_constant_binner();
std::auto_ptr<FieldConfig> make_constant_binner_config();

}
}
}

#endif
