#include <dStorm/outputs/BinnedLocalizations_impl.h>
#include "ImageDiscretizer_inline.h"
#include "LiveCache_inline.h"
#include "TerminalCache_inline.h"
#include "Display_inline.h"
#include "ColourDisplay_impl.h"

namespace dStorm {
namespace outputs {

#define DISC_INSTANCE(Hueing) \
    template class \
        BinnedLocalizations< \
            viewer::Discretizer<\
                viewer::LiveCache<\
                    viewer::Display<\
                        viewer::HueingColorizer<viewer::Hueing> > > > >; \
    template class \
        BinnedLocalizations< \
            viewer::Discretizer<\
                viewer::TerminalCache<\
                    viewer::HueingColorizer<viewer::Hueing> > > > \

DISC_INSTANCE(ColourSchemes::BlackWhite);
DISC_INSTANCE(ColourSchemes::BlackRedYellowWhite);
DISC_INSTANCE(ColourSchemes::FixedHue);
DISC_INSTANCE(ColourSchemes::TimeHue);
DISC_INSTANCE(ColourSchemes::ExtraHue);
DISC_INSTANCE(ColourSchemes::ExtraSaturation);


}
}
