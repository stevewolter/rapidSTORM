#ifndef DSTORM_UNITS_EIGEN_TRAITS_H
#define DSTORM_UNITS_EIGEN_TRAITS_H

#include <Eigen/Core>
#include <boost/units/units_fwd.hpp>
#include <boost/optional/optional.hpp>

namespace Eigen {
    template <typename Unit, typename Scalar>
    struct NumTraits< boost::units::quantity<Unit, Scalar> > : public NumTraits< Scalar > {};
    template <typename Unit, typename Scalar>
    struct NumTraits< const boost::units::quantity<Unit, Scalar> > : public NumTraits< Scalar > {};
}

#endif
