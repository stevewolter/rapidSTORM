#ifndef DSTORM_OUTPUT_BINNING_CONSTANT_H
#define DSTORM_OUTPUT_BINNING_CONSTANT_H

#include "binning/binning_decl.h"
#include "binning/config.h"
#include <memory>

namespace dStorm {
namespace binning {

std::auto_ptr<Unscaled> make_constant_binner();
std::auto_ptr<FieldConfig> make_constant_binner_config();

}
}

#endif
