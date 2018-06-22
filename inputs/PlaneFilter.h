#ifndef DSTORM_INPUT_PLANEFILTER_H
#define DSTORM_INPUT_PLANEFILTER_H

#include "input/fwd.h"
#include <memory>

namespace dStorm {
namespace plane_filter {

std::auto_ptr<input::Link> make_link();

}
}

#endif
