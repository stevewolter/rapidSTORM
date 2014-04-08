#include "density_map/DensityMap.hpp"
#include "density_map/VirtualListener.h"

namespace dStorm {
namespace density_map {

template class DensityMap< VirtualListener<2>, 2 >;
template class DensityMap< VirtualListener<3>, 3 >;

}
}
