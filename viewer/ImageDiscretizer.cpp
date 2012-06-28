#include "ImageDiscretizer_impl.h"
#include "Display_inline.h"
#include "ColourScheme.h"
#include "LiveCache_inline.h"
#include "TerminalBackend.h"
#include <dStorm/image/MetaInfo.h>

namespace dStorm {
namespace viewer {

template class Discretizer< LiveCache< Display< ColourScheme > >, ColourScheme >;
template class Discretizer< TerminalCache, ColourScheme >;

}
}
