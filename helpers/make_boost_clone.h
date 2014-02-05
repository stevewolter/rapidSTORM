#ifndef DSTORM_HELPERS_MAKE_BOOST_CLONE
#define DSTORM_HELPERS_MAKE_BOOST_CLONE

#include <boost/ptr_container/clone_allocator.hpp>

#define MAKE_BOOST_CLONE(Type) \
namespace boost { \
template <> \
inline Type* new_clone<Type>( const Type& t ) { return t.clone(); } \
template <> \
inline void delete_clone<Type>(const Type* t) { delete t; } \
}

#endif
