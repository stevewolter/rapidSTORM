#ifndef DSTORM_INPUT_JOIN_ITERATOR_DECL_HPP
#define DSTORM_INPUT_JOIN_ITERATOR_DECL_HPP

#include <dStorm/input/Source.h>

namespace dStorm {
namespace input {
namespace join {

template <typename Type, typename Tag>
struct merge_traits;

template <typename Type>
struct traits_merger {
    typedef std::auto_ptr< Traits<Type> > result_type;
    typedef std::vector< boost::shared_ptr< const Traits<Type> > > argument_type;
};

template <typename Type, typename Tag>
struct iterator;

}
}
}

#endif
