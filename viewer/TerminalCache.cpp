#include "TerminalCache_impl.h"
#include "Display.h"
#include "ColourDisplay_impl.h"

namespace dStorm {
namespace viewer {

#define DISC_INSTANCE(Hueing) \
        template class TerminalCache< HueingColorizer<Hueing> >

#include "ColourDisplay_instantiations.h"

}

}
