#include "viewer/ImageDiscretizer.hpp"
#include "viewer/LiveCache.h"
#include "viewer/TerminalCache.h"

namespace dStorm {
namespace viewer {

template class Discretizer< LiveCache >;
template class Discretizer< TerminalCache >;

}
}
