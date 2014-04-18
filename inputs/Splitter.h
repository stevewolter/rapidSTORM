#ifndef DSTORM_INPUT_SPLITTER_DECL_H
#define DSTORM_INPUT_SPLITTER_DECL_H

#include "input/fwd.h"
#include <memory>

namespace dStorm {
namespace Splitter {

std::auto_ptr<input::Link> makeLink();

}
}

#endif
