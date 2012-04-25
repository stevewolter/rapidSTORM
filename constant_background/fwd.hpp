#ifndef FITTER_CONSTANT_BACKGROUND_HPP_FWD
#define FITTER_CONSTANT_BACKGROUND_HPP_FWD

#include <boost/units/systems/si/dimensionless.hpp>

namespace dStorm {
namespace constant_background {

struct Amount {
    typedef boost::units::si::dimensionless Unit;
};

struct Expression;
template <typename Num, typename Param> class Computation;

}
}

#endif
