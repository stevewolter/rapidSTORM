#ifndef DSTORM_INPUTS_LOCALIZATIONFILE_H
#define DSTORM_INPUTS_LOCALIZATIONFILE_H

#include <dStorm/input/chain/Link_decl.h>
#include <memory>

namespace dStorm {
namespace inputs {
namespace LocalizationFile {
std::auto_ptr<input::chain::Link> create();
}
}
}

#endif
