#include "DensityMap.hpp"
#include "VirtualListener.h"

namespace dStorm {
namespace density_map {

template class DensityMap< VirtualListener<2>, 2 >;
template class DensityMap< VirtualListener<3>, 3 >;

}
}
