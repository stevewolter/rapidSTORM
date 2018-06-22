#ifndef DSTORM_INPUT_SAMPLEINFO_H
#define DSTORM_INPUT_SAMPLEINFO_H

#include <memory>
#include "input/fwd.h"

namespace dStorm {
namespace input {
namespace sample_info {

std::auto_ptr<Link> makeLink();
class Config;
class Source;
template <typename ForwardedType>
class Input;

}
}
}

#endif
