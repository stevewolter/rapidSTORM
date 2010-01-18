#include "ImageDiscretizer_impl.h"
#include "Display_inline.h"
#include "ColourDisplay_impl.h"

namespace dStorm {
namespace viewer {
namespace DiscretizedImage {

#define DISC_INSTANCE(Hueing) ImageDiscretizer< \
        HueingColorizer<Hueing>, \
        Display< HueingColorizer<Hueing> > >

template class DISC_INSTANCE(ColourSchemes::BlackWhite);
template class DISC_INSTANCE(ColourSchemes::BlackRedYellowWhite);
template class DISC_INSTANCE(ColourSchemes::FixedHue);
template class DISC_INSTANCE(ColourSchemes::TimeHue);
template class DISC_INSTANCE(ColourSchemes::ExtraHue);
template class DISC_INSTANCE(ColourSchemes::ExtraSaturation);

}
}
}
