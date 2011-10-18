#ifndef DSTORM_TYPES_SUBPIXELPOSITION_H
#define DSTORM_TYPES_SUBPIXELPOSITION_H

#include <boost/units/systems/si/length.hpp>
#include <Eigen/Core>

namespace dStorm {

typedef Eigen::Matrix< 
    boost::units::quantity< boost::units::si::length, float >,
    3, 1, Eigen::DontAlign > samplepos;

}

#endif
