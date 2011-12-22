#ifndef DSTORM_INPUT_IMAGEVECTOR_H
#define DSTORM_INPUT_IMAGEVECTOR_H

#include <memory>
#include <dStorm/input/chain/Link_decl.h>

namespace dStorm { 
namespace input_buffer { 

std::auto_ptr<input::chain::Link> makeLink();

}
}

#endif
