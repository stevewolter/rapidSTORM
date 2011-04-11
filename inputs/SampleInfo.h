#ifndef DSTORM_INPUT_SAMPLEINFO_H
#define DSTORM_INPUT_SAMPLEINFO_H

#include <memory>
#include <dStorm/input/chain/Filter_decl.h>

namespace dStorm {
namespace input {
namespace sample_info {

std::auto_ptr<chain::Filter> makeLink();
class Config;
class Source;
template <typename ForwardedType>
class Input;

}
}
}

#endif
