#ifndef DSTORM_TRAITS_DEPTHINFO_H
#define DSTORM_TRAITS_DEPTHINFO_H

#include <dStorm/threed_info/Polynomial3D.h>
#include <dStorm/threed_info/Spline3D.h>
#include "types.h"

namespace dStorm {
namespace threed_info {

struct No3D {
    Sigma sigma[2];
};

typedef boost::variant< Polynomial3D, No3D, Spline3D > DepthInfo;

}
}

#endif
