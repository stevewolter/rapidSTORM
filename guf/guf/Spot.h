#ifndef NONLINFIT_IMAGE_SPOT_H
#define NONLINFIT_IMAGE_SPOT_H

#include <Eigen/Core>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/Eigen/Core>

namespace dStorm {
namespace guf {

typedef Eigen::Matrix< boost::units::quantity<boost::units::si::length, float>, 2, 1, Eigen::DontAlign >
    Spot;

}
}

#endif
