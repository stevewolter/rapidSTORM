#ifndef DSTORM_INPUT_YMIRROR_DECL_H
#define DSTORM_INPUT_YMIRROR_DECL_H

#include <dStorm/input/chain/Link_decl.h>
#include <memory>

namespace dStorm {
namespace YMirror {

class Config;
template <typename Object> 
class Source;
class ChainLink;

std::auto_ptr<input::chain::Link> makeLink();

}
}

#endif
