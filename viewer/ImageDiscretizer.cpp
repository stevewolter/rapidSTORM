#include "ImageDiscretizer_impl.h"
#include "Display_inline.h"
#include "ColourDisplay_impl.h"
#include "LiveCache_inline.h"
#include "TerminalBackend.h"

namespace dStorm {
namespace viewer {

#define DISC_INSTANCE(Hueing) template class Discretizer< \
        LiveCache< Display< HueingColorizer<Hueing> > > >; \
   template class Discretizer< TerminalCache< HueingColorizer<Hueing> > >

DISC_INSTANCE(ColourSchemes::BlackWhite);
DISC_INSTANCE(ColourSchemes::BlackRedYellowWhite);
DISC_INSTANCE(ColourSchemes::FixedHue);
DISC_INSTANCE(ColourSchemes::TimeHue);
DISC_INSTANCE(ColourSchemes::ExtraHue);
DISC_INSTANCE(ColourSchemes::ExtraSaturation);

}
}
