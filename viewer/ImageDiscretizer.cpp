#include "ImageDiscretizer_impl.h"
#include "Display_inline.h"
#include "ColourDisplay_impl.h"
#include "LiveCache_inline.h"
#include "TerminalBackend.h"
#include <dStorm/ImageTraits_impl.h>

namespace dStorm {
namespace viewer {

#define DISC_INSTANCE(Hueing) template class Discretizer< \
        LiveCache< Display< HueingColorizer<Hueing> > > >; \
   template class Discretizer< TerminalCache< HueingColorizer<Hueing> > >

#include "ColourDisplay_instantiations.h"

}
}
