#include "debug.h"
#include "optics.h"
#include "optics_config.h"
#include <Eigen/Core>
#include <Eigen/LU>
#include <Eigen/Geometry>
#include <boost/units/Eigen/Array>
#include <boost/units/cmath.hpp>
#include <functional>

namespace dStorm {
namespace traits {

using namespace boost::units;

Optics::Optics() {}

Optics::~Optics() {}

float traits::Optics::transmission_coefficient( int fluorophore ) const
{
    DEBUG("Getting transmission of " << fluorophore << " of " << tmc.size() << " at " << this);
    if ( int(tmc.size()) > fluorophore )
        return tmc[fluorophore];
    else
        return 1.0f;
}

void traits::Optics::set_fluorophore_transmission_coefficient( int fluorophore, float value ) 
{
    DEBUG("Did set transmission of " << fluorophore << " to " << value << " at " << this);
    while ( int(tmc.size()) <= fluorophore )
        tmc.push_back( 1.0f );
    tmc[fluorophore] = value;
}

}
}
