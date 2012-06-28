#include <dStorm/outputs/BinnedLocalizations_impl.h>
#include "density_map/VirtualListener.h"
#include "Image.h"

namespace dStorm {
namespace outputs {

template class BinnedLocalizations< density_map::VirtualListener<viewer::Im::Dim>, viewer::Im::Dim >;

}
}
