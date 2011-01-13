#include "LiveCache_impl.h"
#include "Display.h"
#include "colour_schemes/impl.h"
#include "Display_impl.h"
#include <dStorm/image/constructors.h>

namespace dStorm {
namespace viewer {

#define DISC_INSTANCE(Hueing) \
        template class LiveCache< Display< Hueing > >

#include "colour_schemes/instantiate.h"

}

template class Image<viewer::HistogramPixel,2>;
}
