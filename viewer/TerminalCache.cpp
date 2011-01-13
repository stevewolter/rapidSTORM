#include "TerminalCache_impl.h"
#include "Display.h"
#include "colour_schemes/impl.h"

namespace dStorm {
namespace viewer {

#define DISC_INSTANCE(Hueing) \
        template class TerminalCache< Hueing >

#include "colour_schemes/instantiate.h"

}

}
