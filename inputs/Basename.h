#ifndef DSTORM_INPUT_BASENAME_DECL_H
#define DSTORM_INPUT_BASENAME_DECL_H

#include <memory>
#include <dStorm/input/chain/Link_decl.h>

namespace dStorm {
namespace basename_input_field {

std::auto_ptr<input::chain::Link> makeLink();
class Config;
class ChainLink;

}
}

#endif
