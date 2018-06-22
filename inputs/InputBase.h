#ifndef DSTORM_CONFIG_INPUTBASE_H
#define DSTORM_CONFIG_INPUTBASE_H

#include <memory>
#include "input/fwd.h"

namespace dStorm {

std::auto_ptr< input::Link > make_input_base();

}

#endif
