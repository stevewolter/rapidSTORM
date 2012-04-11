#ifndef DSTORM_TRAITS_GET_SIGMA_H
#define DSTORM_TRAITS_GET_SIGMA_H

#include "fwd.h"
#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/length.hpp>
#include "DepthInfo.h"
#include "types.h"

namespace dStorm {
namespace threed_info {

Sigma get_sigma( const DepthInfo& o, Direction dir, ZPosition );

}
}

#endif
