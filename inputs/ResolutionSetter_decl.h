#ifndef DSTORM_INPUT_RESOLUTIONSETTER_H
#define DSTORM_INPUT_RESOLUTIONSETTER_H

#include <memory>
#include <dStorm/input/chain/Filter_decl.h>

namespace dStorm {
namespace input {

namespace Resolution {

std::auto_ptr<chain::Filter> makeLink();
class Config;
class ChainLink;
template <typename ForwardedType>
class ResolutionSetter;

}
}
}

#endif
