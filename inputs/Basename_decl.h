#ifndef DSTORM_INPUT_BASENAME_DECL_H
#define DSTORM_INPUT_BASENAME_DECL_H

#include <memory>
#include <dStorm/input/chain/Filter_decl.h>

namespace dStorm {
namespace input {
namespace Basename {

std::auto_ptr<chain::Filter> makeLink();
class Config;
class ChainLink;

}
}
}

#endif
