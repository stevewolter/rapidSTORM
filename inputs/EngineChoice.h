#ifndef DSTORM_CONFIG_ENGINECHOICE_H
#define DSTORM_CONFIG_ENGINECHOICE_H

#include <dStorm/input/chain/Link.h>

namespace dStorm {

std::auto_ptr< input::chain::Link > make_engine_choice();

}

#endif
