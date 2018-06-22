#ifndef DSTORM_INPUTS_LOCALIZATIONFILE_H
#define DSTORM_INPUTS_LOCALIZATIONFILE_H

#include "input/fwd.h"
#include <memory>

namespace dStorm {
namespace inputs {
namespace LocalizationFile {
std::auto_ptr<input::Link> create();
}
}
}

#endif
