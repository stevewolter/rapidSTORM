#ifndef DSTORM_PAIR_EIGEN_TRAITS_H
#define DSTORM_PAIR_EIGEN_TRAITS_H

#include <Eigen/Core>
#include <utility>

namespace Eigen {
    template <typename Numeric>
    struct NumTraits< std::pair<Numeric,Numeric> > : public NumTraits< Numeric > {};
    template <typename Numeric>
    struct NumTraits< const std::pair<Numeric,Numeric> > : public NumTraits< Numeric > {};
}

#endif
