#ifndef DSTORM_TRAITS_NORANGE_H
#define DSTORM_TRAITS_NORANGE_H

#include "base.h"

namespace dStorm {
namespace traits {

template <typename Type>
struct NoRange {
    static const bool has_range = false;

    typedef boost::optional<Type> RangeBoundType;
    typedef std::pair< boost::optional<Type>, boost::optional<Type> > RangeType;

  private:
    static const RangeType static_range;

  public:
    const RangeType& range() const { return static_range; }
    RangeType& range() { throw std::runtime_error("static range cannot be set"); }

    inline bool is_in_range( const Type& t) const;

    inline RangeBoundType lower_limits() const { throw std::logic_error("No lower limits in NoRange class"); }
    inline RangeBoundType upper_limits() const { throw std::logic_error("No lower limits in NoRange class"); }
};

}
}

#endif
