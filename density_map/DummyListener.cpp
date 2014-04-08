#include "density_map/DensityMap.hpp"
#include "density_map/DummyListener.h"

namespace dStorm {
namespace density_map {

template class DensityMap< DummyListener<2>, 2 >;
template class DensityMap< DummyListener<3>, 3 >;

}
}
