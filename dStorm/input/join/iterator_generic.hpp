#ifndef DSTORM_INPUT_JOIN_ITERATOR_GENERIC_HPP
#define DSTORM_INPUT_JOIN_ITERATOR_GENERIC_HPP

#include "iterator.hpp"

namespace dStorm {
namespace input {
namespace join {

template <typename Type, typename Tag>
struct merge_traits : public traits_merger<Type> {
    inline typename traits_merger<Type>::result_type
        operator()( const typename traits_merger<Type>::argument_type& ) 
        { return typename traits_merger<Type>::result_type(); }
};

template <typename Type, typename Tag>
struct merge_data {
    typedef Type result_type;
    inline Type operator()( 
        const input::Traits<Type>& traits,
        const std::vector< typename input::Source<Type>::iterator >& s,
        Tag ) const
    {
        throw std::runtime_error("Joining multiple input files of this type not implemented yet, sorry");
    }
};

}
}
}

#endif
