#ifndef DSTORM_TRAITS_RANGE_H
#define DSTORM_TRAITS_RANGE_H

#include "base.h"
#include "no_range.h"

#include "../unit_interval.h"

namespace dStorm {
namespace traits {

template <typename Base>
class Range 
: public NoRange<Base> {
    typedef NoRange<Base> Types;
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

    inline bool is_in_range( const typename Types::RangeBoundType& t) const;
    inline bool is_in_range( const typename Types::MoS::template value< typename Types::Type >&) const;

    inline typename Types::RangeBoundType lower_limits() const;
    inline typename Types::RangeBoundType upper_limits() const;
    inline typename Types::RangeBoundType width() const;
    inline typename Types::IntervalRangeType as_interval() const;
};

}
}

#endif
