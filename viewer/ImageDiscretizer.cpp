#include "ImageDiscretizer.hpp"
#include "LiveCache.h"
#include "TerminalCache.h"

namespace dStorm {
namespace viewer {

template class Discretizer< LiveCache >;
template class Discretizer< TerminalCache >;

}
}
