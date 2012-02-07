#ifndef DSTORM_ENGINE_FITPOSITION_H
#define DSTORM_ENGINE_FITPOSITION_H

#include <Eigen/Core>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/Eigen/Core>

namespace dStorm {
namespace engine {

typedef Eigen::Matrix< boost::units::quantity<boost::units::si::length, float>, 2, 1, Eigen::DontAlign >
    FitPosition;

}
}

#endif
