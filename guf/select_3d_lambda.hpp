#include "psf/fwd.h"
#include <dStorm/threed_info/DepthInfo.h>

namespace dStorm {
namespace guf {

template <class Traits3D>
class select_3d_lambda;

template <> struct select_3d_lambda<threed_info::Polynomial3D> { typedef PSF::Polynomial3D type; };
template <> struct select_3d_lambda<threed_info::Spline3D> { typedef PSF::Spline3D type; };
template <> struct select_3d_lambda<threed_info::No3D> { typedef PSF::No3D type; };

}
}
