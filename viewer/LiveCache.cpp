#include "LiveCache_impl.h"
#include "Display.h"
#include "ColourDisplay_impl.h"
#include "Display_impl.h"
#include <dStorm/image/constructors.h>

namespace dStorm {
namespace viewer {

#define DISC_INSTANCE(Hueing) \
        LiveCache< Display< HueingColorizer<Hueing> > >

template class DISC_INSTANCE(ColourSchemes::BlackWhite);
template class DISC_INSTANCE(ColourSchemes::BlackRedYellowWhite);
template class DISC_INSTANCE(ColourSchemes::FixedHue);
template class DISC_INSTANCE(ColourSchemes::TimeHue);
template class DISC_INSTANCE(ColourSchemes::ExtraHue);
template class DISC_INSTANCE(ColourSchemes::ExtraSaturation);

}

template class Image<viewer::HistogramPixel,2>;
}
