#include "DensityMap.hpp"
#include "DummyListener.h"

namespace dStorm {
namespace density_map {

template class DensityMap< DummyListener<2>, 2 >;
template class DensityMap< DummyListener<3>, 3 >;

}
}
