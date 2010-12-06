#include "Display_impl.h"
#include "ColourDisplay_impl.h"

namespace dStorm {
namespace viewer {

#define DISC_INSTANCE(Hueing) template class Display< HueingColorizer<Hueing> >

#include "ColourDisplay_instantiations.h"

}
}
