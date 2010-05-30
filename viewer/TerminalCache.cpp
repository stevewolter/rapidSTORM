#include "TerminalCache_impl.h"
#include "Display.h"
#include "ColourDisplay_impl.h"

namespace dStorm {
namespace viewer {

#define DISC_INSTANCE(Hueing) \
        TerminalCache< HueingColorizer<Hueing> >

template class DISC_INSTANCE(ColourSchemes::BlackWhite);
template class DISC_INSTANCE(ColourSchemes::BlackRedYellowWhite);
template class DISC_INSTANCE(ColourSchemes::FixedHue);
template class DISC_INSTANCE(ColourSchemes::TimeHue);
template class DISC_INSTANCE(ColourSchemes::ExtraHue);
template class DISC_INSTANCE(ColourSchemes::ExtraSaturation);

}

}
