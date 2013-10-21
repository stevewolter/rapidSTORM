#ifndef DSTORM_TRAITS_BASE_H
#define DSTORM_TRAITS_BASE_H

#include "../namespaces.h"
#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/dimensionless.hpp>
#include <Eigen/Core>
#include "../pair_Eigen_traits.h"
#include <boost/units/Eigen/Core>
#include <boost/optional/optional.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/numeric/interval/interval.hpp>

namespace dStorm {
namespace traits {

template <typename Type>
struct Value {
  public:
    typedef Type ValueType;
    typedef Type OutputType;
    bool is_given;
    boost::optional<ValueType> static_value;

    Value() : is_given(false) {}
};

}
}

#endif
