#include "LiveCache_impl.h"
#include "Display.h"
#include "ColourScheme.h"
#include "Display_impl.h"
#include <dStorm/image/constructors.h>

namespace dStorm {
namespace viewer {

template class LiveCache< Display< ColourScheme > >;

}
}
