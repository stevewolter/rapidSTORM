#include "LiveCache_impl.h"
#include "Display.h"
#include "ColourDisplay_impl.h"
#include "Display_impl.h"
#include <dStorm/image/constructors.h>

namespace dStorm {
namespace viewer {

#define DISC_INSTANCE(Hueing) \
        template class LiveCache< Display< HueingColorizer<Hueing> > >

#include "ColourDisplay_instantiations.h"

}

template class Image<viewer::HistogramPixel,2>;
}
