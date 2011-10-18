#ifndef DSTORM_INTERVAL_EIGEN_TRAITS_H
#define DSTORM_INTERVAL_EIGEN_TRAITS_H

#include <Eigen/Core>
#include <boost/numeric/interval/interval.hpp>

namespace Eigen {
    template <typename Numeric>
    struct NumTraits< boost::numeric::interval<Numeric> > : public NumTraits< Numeric > {};
}

#endif
