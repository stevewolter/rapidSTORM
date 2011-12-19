#ifndef DSTORM_INPUT_SPLITTER_DECL_H
#define DSTORM_INPUT_SPLITTER_DECL_H

#include <dStorm/input/chain/Link_decl.h>
#include <memory>

namespace dStorm {
namespace Splitter {

class Config;
class Source;
class ChainLink;

std::auto_ptr<input::chain::Link> makeLink();

}
}

#endif
