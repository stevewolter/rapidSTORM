#ifndef DSTORM_TRAITS_EQUIFOCAL_PLANE_H
#define DSTORM_TRAITS_EQUIFOCAL_PLANE_H

#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/length.hpp>

namespace dStorm {
namespace traits {

struct Optics;

boost::units::quantity<boost::units::si::length,float>
    equifocal_plane( const Optics& o );

}
}

#endif
