#ifndef DSTORM_CONFIG_INPUTBASE_H
#define DSTORM_CONFIG_INPUTBASE_H

#include <memory>
#include <dStorm/input/chain/Link_decl.h>

namespace dStorm {

std::auto_ptr< input::chain::Link > make_input_base();

}

#endif
