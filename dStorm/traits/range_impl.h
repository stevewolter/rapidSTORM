#ifndef DSTORM_TRAITS_RANGE_IMPL_H
#define DSTORM_TRAITS_RANGE_IMPL_H

#include "base_apply.h"
#include "../units_Eigen_traits.h"
#include "../pair_Eigen_traits.h"

namespace dStorm {
namespace traits {

template <typename Base>
struct Range<Base>::in_range_functor {
    typedef bool result_type;
    typedef const typename Types::Type first_argument_type;
    typedef const typename Types::BoundPair second_argument_type;

    bool operator()( first_argument_type a, second_argument_type b ) { 
        return ( (!b.first.is_set()) || a >= *b.first ) &&
                ( (!b.second.is_set()) || a <= *b.second );
    }
};

template <typename Base>
struct Range<Base>::upper_limit_functor {
    typedef const boost::optional<typename Types::Type> result_type;
    typedef const typename Types::BoundPair argument_type;

    result_type operator()( argument_type a ) { 
        return a.second;
    }
};

template <typename Base>
struct Range<Base>::width_functor {
    typedef const typename Types::Type result_type;
    typedef const typename Types::BoundPair argument_type;

    result_type operator()( argument_type a ) { 
        if ( a.first.set() && a.second.is_set() )
            return *a.second - *a.first;
        else if ( std::numeric_limits<result_type>::has_infinity )
            return std::numeric_limits<result_type>::infinity();
        else
            return std::numeric_limits<result_type>::max();
    }
};

template <typename Base>
bool Range<Base>::is_in_range(const typename Types::RangeBoundType& t) const
{
    in_range_functor r;
    typedef typename Types::MoS::template value<typename in_range_functor::first_argument_type>::type T1;
    T1& t1 = t;
    typedef typename Types::MoS::template value<typename in_range_functor::second_argument_type>::type T2;
    T2& t2 = range();
    return Types::MoS::apply( r, t1, t2 ).all();
}

template <typename Base>
typename Range<Base>::Types::RangeBoundType
Range<Base>::upper_limits() const
{
    upper_limit_functor r;
    return Types::MoS::apply( r, range() );
}

template <typename Base>
typename Range<Base>::Types::RangeBoundType
Range<Base>::width() const
{
    width_functor r;
    return Types::MoS::apply( r, range() );
}

template <typename Type>
struct scale {
    typedef boost::optional<float> result_type;
    typedef const Type first_argument_type;
    typedef const std::pair< boost::optional<Type>, boost::optional<Type> > second_argument_type;

    result_type operator()( first_argument_type& a, second_argument_type& b ) { 
        if ( b.first.is_initialized() && b.second.is_initialized() )
            return result_type( (a - *b.first) * 1.0f / ( *b.second - *b.first ) );
        else {
            return boost::optional<float>();
        }
    }
};

}
}

#endif
