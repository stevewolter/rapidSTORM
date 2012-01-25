#include "ImageDiscretizer_impl.h"
#include "Display_inline.h"
#include "colour_schemes/impl.h"
#include "LiveCache_inline.h"
#include "TerminalBackend.h"
#include <dStorm/ImageTraits_impl.h>

namespace dStorm {
namespace viewer {

#define DISC_INSTANCE(Hueing) template class Discretizer< \
        LiveCache< Display< Hueing > >, Hueing >; \
   template class Discretizer< TerminalCache< Hueing >, Hueing >

#include "colour_schemes/instantiate.h"

}
}
