#ifndef DSTORM_MAKE_CLONE_ALLOCATOR_HPP
#define DSTORM_MAKE_CLONE_ALLOCATOR_HPP

#include <boost/ptr_container/clone_allocator.hpp>
#define DSTORM_MAKE_BOOST_CLONE_ALLOCATOR(x) \
namespace boost { \
template <> inline x* new_clone<x>( const x& l ) { return l.clone(); } \
template <> inline void delete_clone<x>(const x* l) { delete l; } \
}

#endif
