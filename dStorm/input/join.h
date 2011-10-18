#ifndef DSTORM_INPUT_JOIN_H
#define DSTORM_INPUT_JOIN_H

#include <dStorm/input/chain/Link.h>
#include <memory>

namespace dStorm {
namespace input {
namespace join {

std::auto_ptr<chain::Link> create_link( std::auto_ptr<chain::Link> child );

}
}
}

#endif
