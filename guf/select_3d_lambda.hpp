#include "psf/fwd.h"
#include <dStorm/traits/DepthInfo.h>

namespace dStorm {
namespace guf {

template <class Traits3D>
class select_3d_lambda;

template <> struct select_3d_lambda<traits::Polynomial3D> { typedef PSF::Polynomial3D type; };
template <> struct select_3d_lambda<traits::Spline3D> { typedef PSF::Spline3D type; };
template <> struct select_3d_lambda<traits::No3D> { typedef PSF::No3D type; };

}
}
