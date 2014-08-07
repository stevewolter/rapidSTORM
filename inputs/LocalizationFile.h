#ifndef DSTORM_INPUTS_LOCALIZATIONFILE_H
#define DSTORM_INPUTS_LOCALIZATIONFILE_H

#include <memory>
#include "input/Link.h"
#include "localization/record.h"

namespace dStorm {
namespace inputs {
namespace LocalizationFile {
std::unique_ptr<input::Link<localization::Record>> create();
}
}
}

#endif
