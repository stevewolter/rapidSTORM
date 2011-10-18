#ifndef DSTORM_TRAITS_NORANGE_H
#define DSTORM_TRAITS_NORANGE_H

#include "base.h"

namespace dStorm {
namespace traits {

template <typename Base>
struct NoRange {
    typedef typename value<Base>::Scalar Type;
    typedef std::pair< boost::optional<Type>, boost::optional<Type> > BoundPair;
    typedef typename value<Base>::MoS MoS;
  public:
    static const bool has_range = false;

    typedef typename MoS::template value< boost::optional<Type> >::type RangeBoundType;
    typedef typename MoS::template value< boost::numeric::interval<Type> >::type IntervalRangeType;
    typedef typename MoS::template value< BoundPair >::type RangeType;

  private:
    static const RangeType static_range;

  public:
    const RangeType& range() const { return static_range; }
    RangeType& range() { throw std::runtime_error("static range cannot be set"); }

    inline bool is_in_range( const RangeBoundType& t) const;
    inline bool is_in_range( const typename MoS::template value< Type >& t) const;

    inline RangeBoundType lower_limits() const { throw std::logic_error("No lower limits in NoRange class"); }
    inline RangeBoundType upper_limits() const { throw std::logic_error("No lower limits in NoRange class"); }
    inline IntervalRangeType as_interval() const { throw std::logic_error("No lower limits in NoRange class"); }
};

}
}

#endif
