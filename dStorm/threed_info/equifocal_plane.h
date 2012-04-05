#ifndef DSTORM_TRAITS_EQUIFOCAL_PLANE_H
#define DSTORM_TRAITS_EQUIFOCAL_PLANE_H

#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/length.hpp>
#include "DepthInfo.h"

namespace dStorm {
namespace threed_info {

boost::units::quantity<boost::units::si::length,float>
    equifocal_plane( const DepthInfo& o );

}
}

#endif
