#ifndef DSTORM_INPUT_BASENAME_DECL_H
#define DSTORM_INPUT_BASENAME_DECL_H

#include <memory>
#include "input/fwd.h"

namespace dStorm {
namespace basename_input_field {

std::auto_ptr<input::Link> makeLink();
class Config;
class ChainLink;

}
}

#endif
