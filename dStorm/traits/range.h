#ifndef DSTORM_TRAITS_RANGE_H
#define DSTORM_TRAITS_RANGE_H

#include "base.h"
#include "no_range.h"

#include "../unit_interval.h"

namespace dStorm {
namespace traits {

template <typename Type>
class Range 
: public NoRange<Type> {
    typedef NoRange<Type> Types;
  public:
    static const bool has_range = true;

    struct in_range_functor;
    struct lower_limit_functor;
    struct upper_limit_functor;
    struct width_functor;

  private:
    typename Types::RangeType _range;

  public:
    const typename Types::RangeType& range() const { return _range; }
    typename Types::RangeType& range() { return _range; }

    inline bool is_in_range( const Type& t) const {
      return _range.first <= t && _range.second >= t;
    }

    inline typename Types::RangeBoundType lower_limits() const { return _range.first; }
    inline typename Types::RangeBoundType upper_limits() const { return _range.second; }
    inline typename Types::RangeBoundType width() const { return _range.second - _range.first; }
};

}
}

#endif
